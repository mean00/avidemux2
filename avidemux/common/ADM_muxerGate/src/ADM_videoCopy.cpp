/**
    \file ADM_videoCopy
    \brief Wrapper 
    (c) Mean 2008/GPLv2

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_videoCopy.h"
#include "ADM_editor/ADM_edit.hxx"

extern ADM_Composer *video_body; // Fixme!
/**
    \fn ADM_videoStreamCopy
*/
ADM_videoStreamCopy::ADM_videoStreamCopy(uint64_t startTime,uint64_t endTime)
{
    aviInfo info;
    video_body->getVideoInfo(&info);
    width=info.width;
    height=info.height;
    fourCC=info.fcc;
    averageFps1000=info.fps1000;
    isCFR=false;
    // Estimate start frame
    currentFrame= video_body->searchFrameBefore(startTime+1);
    while(currentFrame)
    {
        uint32_t flags;
        video_body->getFlags(currentFrame,&flags);
        if(flags&AVI_KEY_FRAME) break;
        currentFrame--;
    }
    eofMet=false;
    this->startTime=startTime;
    this->endTime=endTime;
    // Update start time if needed
    uint64_t sPts,sDts,sStart=ADM_NO_PTS;
    video_body->getPtsDts(currentFrame,&sPts,&sDts);
    if(sDts!=ADM_NO_PTS) sStart=sDts;
    else
        if(sPts!=ADM_NO_PTS)
        {
            sStart=sPts;
            printf("[Warning] No Dts available for first frame, guessing ...\n");
        }
    if(sStart!=ADM_NO_PTS)
        this->startTime=sStart;
    printf("[StreamCopy] Fixating start time by %u\n",abs((int)(this->startTime-startTime)));
}
/**
    \fn ADM_videoStreamCopy
*/
ADM_videoStreamCopy::~ADM_videoStreamCopy()
{

}
/**
    \fn getExtraData
*/
bool     ADM_videoStreamCopy::getExtraData(uint32_t *extraLen, uint8_t **extraData)
{

  return video_body->getExtraHeaderData(extraLen,extraData);
}
/**
    \fn rescaleTs
*/
uint64_t  ADM_videoStreamCopy::rescaleTs(uint64_t in)
{
    if(in==ADM_NO_PTS) return in;
    if(in>startTime) return in-startTime;
    return 0;
}
/**
    \fn getStartTime
*/
uint64_t  ADM_videoStreamCopy::getStartTime(void)
{
    return this->startTime;
}
/**
    \fn getPacket
*/
bool  ADM_videoStreamCopy::getPacket(uint32_t *len, uint8_t *data, uint32_t maxLen,
                    uint64_t *pts,uint64_t *dts,
                    uint32_t *flags)
{
    if(true==eofMet) return false;
    image.data=data;
    if(false==video_body->getCompressedPicure(currentFrame,&image))
    {
            printf("[StreamCopy] Get packet failed for frame %d\n",currentFrame);
            return false;
    }
    *len=image.dataLength;
    ADM_assert(*len<maxLen);
    
    *pts=rescaleTs(image.demuxerPts);
    *dts=rescaleTs(image.demuxerDts);
    if(image.demuxerDts!=ADM_NO_PTS)
    {
        if(image.demuxerDts>endTime ) 
        {
            eofMet=true;
            return false;
        }   
    }
    *flags=image.flags;
    currentFrame++;
    return true;
}
/**
    \fn getVideoDuration
*/
uint64_t        ADM_videoStreamCopy::getVideoDuration(void)
{
    return video_body->getVideoDuration();

}
/**

*/
bool     ADM_videoStreamCopy::providePts(void)
{
    return true;
}
