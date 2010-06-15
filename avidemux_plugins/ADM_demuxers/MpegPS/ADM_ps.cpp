/***************************************************************************
    copyright            : (C) 2007/2009 by mean
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
#include "fourcc.h"
#include "DIA_coreToolkit.h"
#include "ADM_indexFile.h"
#include "ADM_ps.h"

#include <math.h>

uint32_t ADM_UsecFromFps1000(uint32_t fps1000);

/**
      \fn open
      \brief open the flv file, gather infos and build index(es).
*/

uint8_t psHeader::open(const char *name)
{
    char *idxName=(char *)alloca(strlen(name)+4);
    bool r=false;
    FP_TYPE appendType=FP_DONT_APPEND;
    uint32_t append;
    char *type;
    uint64_t startDts;

    sprintf(idxName,"%s.idx2",name);
    indexFile index;
    if(!index.open(idxName))
    {
        printf("[psDemux] Cannot open index file %s\n",idxName);
        return false;
    }
    if(!index.readSection("System"))
    {
        printf("[psDemux] Cannot read system section\n");
        goto abt;
    }
    type=index.getAsString("Type");
    if(!type || type[0]!='P')
    {
        printf("[psDemux] Incorrect or not found type\n");
        goto abt;
    }
    append=index.getAsUint32("Append");
    printf("[psDemux] Append=%"LU"\n",append);
    if(append) appendType=FP_APPEND;
    if(!parser.open(name,&appendType))
    {
        printf("[psDemux] Cannot open root file %s\n",name);
        goto abt;
    }
    if(!readVideo(&index)) 
    {
        printf("[psDemux] Cannot read Video section of %s\n",idxName);
        goto abt;
    }
    if(!readAudio(&index,name)) 
    {
        printf("[psDemux] Cannot read Audio section of %s => No audio\n",idxName);
    }
    if(!readIndex(&index))
    {
        printf("[psDemux] Cannot read index for file %s\n",idxName);
        goto abt;
    }
    updatePtsDts();
    _videostream.dwLength= _mainaviheader.dwTotalFrames=ListOfFrames.size();
    printf("[psDemux] Found %d video frames\n",_videostream.dwLength);
    if(_videostream.dwLength)_isvideopresent=1;
//***********
    
    psPacket=new psPacketLinear(0xE0);
    if(psPacket->open(name,appendType)==false) 
    {
        printf("psDemux] Cannot psPacket open the file\n");
        goto abt;
    }
    r=true;
    for(int i=0;i<listOfAudioTracks.size();i++)
    {
        ADM_psTrackDescriptor *desc=listOfAudioTracks[i];
        ADM_audioStream *audioStream=ADM_audioCreateStream(&desc->header,desc->access);
        if(!audioStream)
        {
            
        }else       
        {
                desc->stream=audioStream;
        }
    }
abt:
    index.close();
    printf("[psDemuxer] Loaded %d\n",r);
    return r;
}
/**
        \fn getVideoDuration
        \brief Returns duration of video in us
*/
uint64_t psHeader::getVideoDuration(void)
{
    int index=ListOfFrames.size();
    if(!index) return 0;
    index--;
    int offset=0;
    do
    {
        if(ListOfFrames[index]->dts!=ADM_NO_PTS) break;
        index--;
        offset++;

    }while(index);
    if(!index)
    {
        ADM_error("Cannot find a valid DTS in the file\n");
        return 0;
    }
    float f,g;
    f=1000*1000*1000;
    f/=_videostream.dwRate; 
    g=ListOfFrames[index]->dts;
    g+=f*offset;
    return (uint64_t)g;
}


/**
    \fn getAudioInfo
    \brief returns wav header for stream i (=0)
*/
WAVHeader *psHeader::getAudioInfo(uint32_t i )
{
        if(!listOfAudioTracks.size()) return NULL;
      ADM_assert(i<listOfAudioTracks.size());
      return listOfAudioTracks[i]->stream->getInfo();
      
}
/**
   \fn getAudioStream
*/

uint8_t   psHeader::getAudioStream(uint32_t i,ADM_audioStream  **audio)
{
    if(!listOfAudioTracks.size())
    {
            *audio=NULL;
            return true;
    }
  ADM_assert(i<listOfAudioTracks.size());
  *audio=listOfAudioTracks[i]->stream;
  return true; 
}
/**
    \fn getNbAudioStreams

*/
uint8_t   psHeader::getNbAudioStreams(void)
{
 
  return listOfAudioTracks.size(); 
}
/*
    __________________________________________________________
*/

void psHeader::Dump(void)
{
 
}
/**
    \fn close
    \brief cleanup
*/

uint8_t psHeader::close(void)
{
    // Destroy index
    int nb=ListOfFrames.size();
    for(int i=0;i<nb;i++)
    {
        if(ListOfFrames[i]) delete ListOfFrames[i];
        ListOfFrames[i]=0;
    }
    ListOfFrames.clear();
    if(psPacket)
    {
        psPacket->close();
        delete psPacket;
        psPacket=NULL;
    }
    return 1;
}
/**
    \fn psHeader
    \brief constructor
*/

 psHeader::psHeader( void ) : vidHeader()
{ 
    interlaced=false;
    lastFrame=0xffffffff;
    
}
/**
    \fn psHeader
    \brief destructor
*/

 psHeader::~psHeader(  )
{
  close();
}


/**
    \fn setFlag
    \brief Returns timestamp in us of frame "frame" (PTS)
*/

  uint8_t  psHeader::setFlag(uint32_t frame,uint32_t flags)
{
   
     uint32_t f=2;
     if(flags & AVI_KEY_FRAME) f=1;
     if(flags & AVI_B_FRAME) f=3;
     if(frame>=ListOfFrames.size()) return 0;
      ListOfFrames[frame]->type=f;
    return 1;
}
/**
    \fn getFlags
    \brief Returns timestamp in us of frame "frame" (PTS)
*/

uint32_t psHeader::getFlags(uint32_t frame,uint32_t *flags)
{
    if(frame>=ListOfFrames.size()) return 0;
    uint32_t f=ListOfFrames[frame]->type;
    switch(f)
    {
        case 1: *flags=AVI_KEY_FRAME;break;
        case 2: *flags=0;break;
        case 3: *flags=AVI_B_FRAME;break;
    }
    return  1;
}

/**
    \fn getTime
    \brief Returns timestamp in us of frame "frame" (PTS)
*/
uint64_t psHeader::getTime(uint32_t frame)
{
   if(frame>=ListOfFrames.size()) return 0;
    uint64_t pts=ListOfFrames[frame]->pts;
    return pts;
}
/**
    \fn timeConvert
    \brief FIXME
*/
uint64_t psHeader::timeConvert(uint64_t x)
{
    if(x==ADM_NO_PTS) return ADM_NO_PTS;
    x=x-ListOfFrames[0]->dts;
    x=x*1000;
    x/=90;
    return x;

}
/**
        \fn getFrame
*/

uint8_t  psHeader::getFrame(uint32_t frame,ADMCompressedImage *img)
{
    if(frame>=ListOfFrames.size()) return 0;
    dmxFrame *pk=ListOfFrames[frame];
    if(frame==(lastFrame+1) && pk->type!=1)
    {
        lastFrame++;
        bool r=psPacket->read(pk->len,img->data);
             img->dataLength=pk->len;
             img->demuxerFrameNo=frame;
             img->demuxerDts=pk->dts;
             img->demuxerPts=pk->pts;
             //printf("[>>>] %d:%02x %02x %02x %02x\n",frame,img->data[0],img->data[1],img->data[2],img->data[3]);
             getFlags(frame,&(img->flags));
             return r;
    }
    if(pk->type==1)
    {
        if(!psPacket->seek(pk->startAt,pk->index)) return false;
         bool r=psPacket->read(pk->len,img->data);
             img->dataLength=pk->len;
             img->demuxerFrameNo=frame;
             img->demuxerDts=pk->dts;
             img->demuxerPts=pk->pts;
             getFlags(frame,&(img->flags));
             //printf("[>>>] %d:%02x %02x %02x %02x\n",frame,img->data[0],img->data[1],img->data[2],img->data[3]);
             lastFrame=frame;
             return r;

    }
    printf(" [PsDemux] lastFrame :%d this frame :%d\n",lastFrame,frame);
    return false;
}
/**
        \fn getExtraHeaderData
*/
uint8_t  psHeader::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
                *len=0; //_tracks[0].extraDataLen;
                *data=NULL; //_tracks[0].extraData;
                return true;            
}

/**
      \fn getFrameSize
      \brief return the size of frame frame
*/
uint8_t psHeader::getFrameSize (uint32_t frame, uint32_t * size)
{
    if(frame>=ListOfFrames.size()) return 0;
    *size=ListOfFrames[frame]->len;
    return true;
}

/**
    \fn getPtsDts
*/
bool    psHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
    if(frame>=ListOfFrames.size()) return false;
    dmxFrame *pk=ListOfFrames[frame];

    *dts=pk->dts;
    *pts=pk->pts;
    return true;
}
/**
        \fn setPtsDts
*/
bool    psHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
      if(frame>=ListOfFrames.size()) return false;
    dmxFrame *pk=ListOfFrames[frame];

    pk->dts=dts;
    pk->pts=pts;
    return true;


}


//EOF
