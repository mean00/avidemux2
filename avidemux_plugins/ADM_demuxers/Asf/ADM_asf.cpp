/** *************************************************************************
    \file ADM_asf.cpp
    \brief ASF/WMV demuxer
    copyright            : (C) 2006/2009 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_Video.h"
#include "ADM_assert.h"

#include "fourcc.h"
#include "DIA_coreToolkit.h"
#include "ADM_asf.h"
#include "ADM_asfPacket.h"

#if 1
#define aprintf printf
#else
#define aprintf(...) {}
#endif

/**
    \fn getAudioInfo
*/
WAVHeader *asfHeader::getAudioInfo(uint32_t i )
{
  if(!_nbAudioTrack) return NULL;
  
  ADM_assert(i<_nbAudioTrack);
  if(!_audioAccess) return NULL;
  return &(_allAudioTracks[i].wavHeader);
}
/**
    \fn getAudioStream
*/
uint8_t    asfHeader::getAudioStream(uint32_t i,ADM_audioStream  **audio)
{
 *audio=NULL;
  if(!_nbAudioTrack) return true;
  ADM_assert(i<_nbAudioTrack); 
  *audio=_audioStreams[i];
 return 1; 
}
/**
    \fn getNbAudioStreams
*/
uint8_t                 asfHeader::getNbAudioStreams(void)
{
    return _nbAudioTrack;
}

/**
    \fn Dump
*/

void asfHeader::Dump(void)
{
 
  printf("*********** ASF INFO***********\n");
}

/**
    \fn close
*/

uint8_t asfHeader::close(void)
{
	if (_fd) 
		fclose(_fd);

	_fd=NULL;

  if(_videoExtraData)
  {
    delete [] _videoExtraData;
    _videoExtraData=NULL; 
  }
  if(myName)
  {
    delete myName;
    myName=NULL; 
  }
  if(_videoExtraData)
  {
    delete [] _videoExtraData;
    _videoExtraData=NULL; 
  }

  if(_packet)
    delete _packet;
  _packet=NULL;
  
  for(int i=0;i<_nbAudioTrack;i++)
  {
    asfAudioTrak *trk=&(_allAudioTracks[i]);
    if(trk->extraData) delete [] trk->extraData;
    trk->extraData=NULL;
    delete    _audioAccess[i];
    _audioAccess[i]=NULL;
    delete _audioStreams[i];
    _audioStreams[i]=NULL;    
  }
  freeQueue(&readQueue);
  freeQueue(&storageQueue);
  return 1;
}
/**
    \fn asfHeader
*/


 asfHeader::asfHeader( void ) : vidHeader()
{
  _fd=NULL;
  _videoIndex=-1;
  myName=NULL;
  _packetSize=0;
  _videoStreamId=0;
  nbImage=0;
  
  _packet=NULL;
  _nbPackets=0;
  memset(&(_allAudioTracks[0]),0,sizeof(_allAudioTracks));
  memset(&(_audioAccess[0]),0,sizeof(_audioAccess));
  memset(&(_audioStreams[0]),0,sizeof(_audioStreams));
  _nbAudioTrack=0;
  _videostream.dwRate=0;
  _shiftUs=0;

}
/**
    \fn ~ asfHeader
*/

 asfHeader::~asfHeader(  )
{
  close();
}
/**
    \fn open
*/

uint8_t asfHeader::open(const char *name)
{
  _fd=ADM_fopen(name,"rb");
  if(!_fd)
  {
    GUI_Error_HIG("File Error.","Cannot open file\n");
    return 0; 
  }
  myName=ADM_strdup(name);
  if(!getHeaders())
  {
    return 0; 
  }
  ADM_info("Stream Video: index=%d, sid=%d\n",(int)_videoIndex,(int)_videoStreamId);
  for(int i=0;i<_nbAudioTrack;i++)
    ADM_info("Stream Audio: index=%d, sid=%d\n",
                (int)_allAudioTracks[i].streamIndex,(int)_allAudioTracks[i].streamIndex);
  buildIndex();
  fseeko(_fd,_dataStartOffset,SEEK_SET);
  _packet=new asfPacket(_fd,_nbPackets,_packetSize,&readQueue,&storageQueue,_dataStartOffset);
  curSeq=1;
  for(int i=0;i<_nbAudioTrack;i++)
  {
        _audioAccess[i]=new asfAudioAccess(this,i);
        _audioStreams[i]=ADM_audioCreateStream(&(_allAudioTracks[i].wavHeader), _audioAccess[i]);
  }
  if(!nbImage)
    {
        ADM_error("No image found \n");
        return 0;
    }
  return 1;
}
/**
    \fn setFlag
*/

  uint8_t  asfHeader::setFlag(uint32_t frame,uint32_t flags)
{
  ADM_assert(frame<nbImage);
  _index[frame].flags=flags;
  return 1; 
}
/**
    \fn getFlags
*/

uint32_t asfHeader::getFlags(uint32_t frame,uint32_t *flags)
{
  if(frame>=nbImage) return 0;
  if(!frame) *flags=AVI_KEY_FRAME;
  else 
      *flags=_index[frame].flags;
  return 1; 
}
/**
    \fn getFrameSize
*/ 
uint8_t     asfHeader::getFrameSize(uint32_t frame,uint32_t *size)
{
    *size=0;
    if(frame>=nbImage) return 0;
    *size=_index[frame].frameLen;
    return true;
}
/**
    \fn getTime
*/
uint64_t                   asfHeader::getTime(uint32_t frameNum)
{
     if(frameNum>=nbImage) return 0;
     return _index[frameNum].pts; // ??PTS??
}
/**
    \fn getTime
*/

uint64_t                   asfHeader::getVideoDuration(void)
{
    return _duration;
}
/**
    \fn getTime
*/

bool                       asfHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
    if(frame>=nbImage) return false;
    *pts=_index[frame].pts;
    *dts=_index[frame].dts;
    return true;
}
/**
    \fn getTime
*/

bool                       asfHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
     if(frame>=nbImage) return false;
    _index[frame].pts=pts;
    _index[frame].dts=dts;;
    return true;
}
/**
    \fn getFrame
*/    
uint8_t  asfHeader::getFrame(uint32_t framenum,ADMCompressedImage *img)
{
  img->dataLength=0;
  img->flags=AVI_KEY_FRAME;
  uint32_t len=0;
  if(framenum>=nbImage)
  {
    printf("[ASF] Going out of bound %u %u\n",framenum, nbImage);
    return 0;
  }
  if(!_index[framenum].frameLen)
  {
    goto gotcha;
  }
  // In case curSeq is stored as one byte..
  curSeq&=0xff;
  //
  
  aprintf("Framenum %u len: %u curSeq %u frameSeq=%u packetnb=%u \n",
         framenum,_index[framenum].frameLen,curSeq,
         _index[framenum].segNb,_index[framenum].packetNb);
  // Seeking ?
  if(_index[framenum].segNb!=curSeq)
  {
    printf("Seeking.. curseq:%u wanted seq:%u packet=%d\n",curSeq,_index[framenum].segNb,_index[framenum].packetNb);
    if(!_packet->goToPacket(_index[framenum].packetNb))
    {
      printf("[ASF] Cannot seek to frame %u\n",framenum);
      return 0; 
    }
    _packet->purge();
    curSeq=_index[framenum].segNb;
    printf("Seeking done, starting at seq=%u\n",curSeq);
  }
  
  
  len=0;
  uint32_t delta;
  while(1)
  {
    while(readQueue.size())
    {
      asfBit *bit;
      bit=readQueue.front();
      readQueue.pop_front();
      aprintf(">found packet of size %d seq %d, while curseq =%d wanted seg=%u current offset=%u, packet=%u\n",
                    bit->len,bit->sequence,curSeq,_index[framenum].segNb,len,bit->packet);
      // Here it is tricky
      // if len==0 we are reading the first part of our frame
      // The packet may starts with segments from the previous frame
      // discard them
      // Delta is just a security as it should be slightly >10
      delta=256+bit->sequence-_index[framenum].segNb;
      delta &=0xff;
      if(!len) // Starting a new frame
      {
          if(_index[framenum].segNb != bit->sequence )
          {
            aprintf("Dropping seq=%u too old for %u delta %d\n",
                  bit->sequence,_index[framenum].segNb,delta);
            storageQueue.push_back(bit);
            bit=NULL;
            if(delta<230)
            {
              printf("[ASF] Very suspicious delta :%"LU"\n",delta);
            }
            continue; 
          }
          // We have found our first chunk
          curSeq=bit->sequence;
          memcpy(img->data,bit->data,bit->len);
          len=bit->len;
          delete[] bit->data;
          delete bit;
          continue;
      }
      // Continuing a frame
      // If the seq number is different it is the beginning of a new frame
      if(bit->sequence!=curSeq )
      {
        aprintf("New sequence %u->%u while loading %u frame\n",curSeq,bit->sequence,framenum);
        img->dataLength=len;
        readQueue.push_front(bit);
        curSeq=bit->sequence;
        goto gotcha;
      }
      // still same sequence ...add
      memcpy(img->data+len,bit->data,bit->len);
      len+=bit->len;
      storageQueue.push_back(bit);
      bit=NULL;
    }
    if(!_packet->nextPacket(_videoStreamId))
    {
      printf("[ASF] Packet Error\n");
      return 0; 
    }
    _packet->skipPacket();
  }
gotcha:
  
  img->dataLength=len;
  img->demuxerDts=_index[framenum].dts;
  img->demuxerPts=_index[framenum].pts;
  if(len!=_index[framenum].frameLen)
  {
    ADM_error("[ASF] Frame=%u :-> Mismatch found len : %u expected %u\n",framenum,len, _index[framenum].frameLen);
  }
  aprintf(">>Len %d seq %d\n",len,curSeq);
  return 1; 
}
//EOF
