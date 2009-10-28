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
    uint64_t ptsStart=startTime+1;
    uint64_t dtsStart;
    ADM_info("Creating copy video stream\n");
    video_body->getVideoInfo(&info);
    width=info.width;
    height=info.height;
    fourCC=info.fcc;
    averageFps1000=info.fps1000;
    isCFR=false;
    // Estimate start frame
    if(false==video_body->getPKFramePTS(&ptsStart))
    {
        ADM_warning("Cannot find previous keyframe\n");
        ptsStart=dtsStart=startTime;
    }else   
    {
        // Now search the DTS associated with it...
        dtsStart=ptsStart;
        if(false==video_body->getDtsFromPts(&dtsStart))
        {
                ADM_warning("Cannot get DTS for PTS=%"LLU" ms, expect problems\n",ptsStart/1000);
                dtsStart=ptsStart;
        }else
        {
            ADM_info("Using %"LLU" ms as startTime\n",dtsStart/1000);
        }
    }
    eofMet=false;

    this->startTimeDts=dtsStart;
    this->startTimePts=ptsStart;
    this->endTimePts=endTime;

    video_body->goToIntraTimeVideo(ptsStart);
    ADM_info(" Fixating start time by %d\n",abs((int)(startTime-startTimeDts)));
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
    if(in>startTimeDts) return in-startTimeDts;
    ADM_warning("Negative time!\n");
    return 0;
}
/**
    \fn getStartTime
*/
uint64_t  ADM_videoStreamCopy::getStartTime(void)
{
    return this->startTimeDts;
}
/**
    \fn getPacket
*/
bool  ADM_videoStreamCopy::getPacket(uint32_t *len, uint8_t *data, uint32_t maxLen,
                    uint64_t *pts,uint64_t *dts,
                    uint32_t *flags)
{
    if(true==eofMet) return false;
again:
    image.data=data;
    if(false==video_body->getCompressedPicture(&image))
    {
            ADM_warning(" Get packet failed ");
            return false;
    }
    *len=image.dataLength;
    ADM_assert(*len<maxLen);
    if(image.demuxerPts!=ADM_NO_PTS)
        if(image.demuxerPts<startTimePts)   
        {
            if(image.flags & AVI_B_FRAME) 
            {
                ADM_warning("Dropping orphean B frame (PTS=%"LLU" ms)\n",image.demuxerPts/1000);
                goto again;
            }
        }
    *pts=rescaleTs(image.demuxerPts);
    *dts=rescaleTs(image.demuxerDts);
    if(image.demuxerPts!=ADM_NO_PTS)
    {
          if(image.demuxerDts!=ADM_NO_PTS)
          {
            if(image.demuxerPts<image.demuxerDts)
              {
                ADM_warning("PTS<DTS : PTS=%"LLU" ms , DTS=%"LLU"ms\n",image.demuxerPts/1000,image.demuxerDts/1000);

              }

          }
        if(image.demuxerPts>endTimePts ) 
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
