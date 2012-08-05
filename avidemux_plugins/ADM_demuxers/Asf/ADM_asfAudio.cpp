/***************************************************************************
    copyright            : (C) 2006 by mean
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

#include "fourcc.h"
#include "ADM_asf.h"
/**
    \fn getDurationInUs
*/
uint64_t  asfAudioAccess::getDurationInUs(void)
{
    // could use that : father->getVideoDuration()...
    return 0;
}
/**
    \fn asfAudioAccess
*/

asfAudioAccess::~asfAudioAccess()
{
	printf("[asfAudio] Destroying track\n");

	fclose(_fd);
	_fd = NULL;


	delete _packet;

  freeQueue(&readQueue);
  freeQueue(&storageQueue);

	_packet = NULL;
}

/**
    \fn asfAudioAccess
*/
                               
asfAudioAccess::asfAudioAccess(asfHeader *father,uint32_t myRank)
{
  printf("[asfAudio] Creating track\n");
    _myRank=myRank;
    _father=father;
    _track=&(_father->_allAudioTracks[myRank]);
    extraDataLen=_track->extraDataLen;
    extraData= _track->extraData;
    _streamId=_track->streamIndex;
    _dataStart=_father->_dataStartOffset;
    _fd=ADM_fopen(_father->myName,"rb");
    ADM_assert(_fd);
    fseeko(_fd,_dataStart,SEEK_SET);
    _packetSize=_father->_packetSize;
    _packet=new asfPacket(_fd,_father->_nbPackets,_packetSize,
                          &readQueue,&storageQueue,_dataStart);
    _seekPoints=&(_father->audioSeekPoints[myRank]);
    printf("[asfAudio] Length %u\n",getLength());
}
/**
    \fn getPos
*/
uint64_t  asfAudioAccess::getPos(void)
{
    return 0;
}

/**
    \fn setPos
*/

bool   asfAudioAccess::setPos(uint64_t newoffset)
{
  return false; 
}

/**
    \fn goToTime
*/

bool   asfAudioAccess::goToTime(uint64_t dts_us) // PTS!
{
    dts_us+=_father->getShift();
    // Search
    int size=_seekPoints->size();
    if(dts_us<=(*_seekPoints)[0].pts || size<2)
    {
          return setPos( 0);
    }

    for(int i=size-2;i>=0;i--)
    {
        if(dts_us>=(*_seekPoints)[i].pts && dts_us<(*_seekPoints)[i+1].pts)
        {
            return _packet->goToPacket( (*_seekPoints)[i].packetNb);
        }
    }
    return false;
}

/**
    \fn getPacket

*/
bool  asfAudioAccess::getPacket(uint8_t *dest, uint32_t *len, uint32_t maxSize,uint64_t *dts)
{
  *len=0;
  uint32_t delta;
  uint8_t r;
  uint64_t shift=_father->getShift();
  while(1)
  {
   
    while(readQueue.size())
    {
      asfBit *bit;
      bit=readQueue.front();
      readQueue.pop_front();
      // still same sequence ...add
      memcpy(dest,bit->data,bit->len);
      *len=bit->len;
      *dts=bit->pts; // for audio PTS=DTS...
      if(*dts>shift) *dts-=shift;
        else
        {
            ADM_error("ASF audio : Cannot shift, DTS=%"PRIu64", shift=%"PRIu64"\n",*dts,shift);
            *dts=ADM_NO_PTS;
        }
      storageQueue.push_back(bit);
      bit=NULL;
      return 1;
    }
    r=_packet->nextPacket(_streamId);
    _packet->skipPacket();
    if(!r)
    {
      printf("[ASF] Audio Packet Error\n");
      return 0; 
    }
    
   // _packet->skipPacket();
  }
  
  return 0; 
}
//EOF
