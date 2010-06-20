/***************************************************************************
    \file   ADM_coreDemuxerMpegTemplate.cpp
    \brief  Common part for mpeg demuxer, template like but using define
    \author (C) 2010 by mean  : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MY_CLASS
    #error  MY_CLASS not defined
#endif

/**
        \fn getVideoDuration
        \brief Returns duration of video in us
*/
uint64_t MY_CLASS::getVideoDuration(void)
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
WAVHeader *MY_CLASS::getAudioInfo(uint32_t i )
{
        if(!listOfAudioTracks.size()) return NULL;
      ADM_assert(i<listOfAudioTracks.size());
      return listOfAudioTracks[i]->stream->getInfo();
      
}

/**
   \fn getAudioStream
*/

uint8_t   MY_CLASS::getAudioStream(uint32_t i,ADM_audioStream  **audio)
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
uint8_t   MY_CLASS::getNbAudioStreams(void)
{
 
  return listOfAudioTracks.size(); 
}

/**
    \fn setFlag
    \brief Returns timestamp in us of frame "frame" (PTS)
*/

uint8_t  MY_CLASS::setFlag(uint32_t frame,uint32_t flags)
{
    if(frame>=ListOfFrames.size()) return 0;
     uint32_t f=2;
     uint32_t intra=flags & AVI_FRAME_TYPE_MASK;
     if(intra & AVI_KEY_FRAME) f=1;
     if(intra & AVI_B_FRAME) f=3;
     
      ListOfFrames[frame]->type=f;
      ListOfFrames[frame]->pictureType=flags & AVI_STRUCTURE_TYPE_MASK;
    return 1;
}
/**
    \fn getFlags
    \brief Returns timestamp in us of frame "frame" (PTS)
*/

uint32_t MY_CLASS::getFlags(uint32_t frame,uint32_t *flags)
{
    if(frame>=ListOfFrames.size()) return 0;
    uint32_t f=ListOfFrames[frame]->type;
    switch(f)
    {
        case 1: *flags=AVI_KEY_FRAME;break;
        case 2: *flags=0;break;
        case 3: *flags=AVI_B_FRAME;break;
    }
    *flags=*flags+ListOfFrames[frame]->pictureType;
    return  1;
}

/**
    \fn getTime
    \brief Returns timestamp in us of frame "frame" (PTS)
*/
uint64_t MY_CLASS::getTime(uint32_t frame)
{
   if(frame>=ListOfFrames.size()) return 0;
    uint64_t pts=ListOfFrames[frame]->pts;
    return pts;
}

/**
    \fn timeConvert
    \brief FIXME
*/
uint64_t MY_CLASS::timeConvert(uint64_t x)
{
    if(x==ADM_NO_PTS) return ADM_NO_PTS;
    x=x-ListOfFrames[0]->dts;
    x=x*1000;
    x/=90;
    return x;
}

/**
        \fn getExtraHeaderData
*/
uint8_t  MY_CLASS::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
                *len=0; //_tracks[0].extraDataLen;
                *data=NULL; //_tracks[0].extraData;
                return true;            
}

/**
      \fn getFrameSize
      \brief return the size of frame frame
*/
uint8_t MY_CLASS::getFrameSize (uint32_t frame, uint32_t * size)
{
    if(frame>=ListOfFrames.size()) return 0;
    *size=ListOfFrames[frame]->len;
    return true;
}

/**
    \fn getPtsDts
*/
bool    MY_CLASS::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
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
bool    MY_CLASS::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
      if(frame>=ListOfFrames.size()) return false;
    dmxFrame *pk=ListOfFrames[frame];

    pk->dts=dts;
    pk->pts=pts;
    return true;


}

// EOF