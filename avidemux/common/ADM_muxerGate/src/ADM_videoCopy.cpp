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
#include "ADM_cpp.h"
using std::string;
#include "ADM_default.h"
#include "ADM_videoCopy.h"
#include "ADM_edit.hxx"
#include "ADM_coreUtils.h"
#include "ADM_vidMisc.h"
extern ADM_Composer *video_body; // Fixme!

#if 1
#define aprintf ADM_info
#else
#define aprintf(...) {}
#endif

/**
    \fn ADM_videoStreamCopy
*/
ADM_videoStreamCopy::ADM_videoStreamCopy(uint64_t startTime,uint64_t endTime)
{
    aviInfo info;
    uint64_t ptsStart=startTime+1;
    uint64_t dtsStart;
    ADM_info("Creating copy video stream, start time: %s\n",ADM_us2plain(startTime));
    ADM_info("End time: %s\n",ADM_us2plain(endTime));
    video_body->getVideoInfo(&info);
    width=info.width;
    height=info.height;
    fourCC=info.fcc;
    averageFps1000=info.fps1000;
    video_body->getTimeBase(&timeBaseNum,&timeBaseDen,true);
    frameIncrement=video_body->getFrameIncrement(true);
    isCFR=false;
    // Estimate start frame
    if(false==video_body->getPKFramePTS(&ptsStart))
    {
        ADM_warning("Cannot find previous keyframe\n");
        ptsStart=dtsStart=startTime;
    }else   
    {
        uint64_t delta=0;
        video_body->getPtsDtsDelta(ptsStart,&delta);
        ADM_info("PTS/DTS delta=%" PRIu64" us\n",delta);
        // Do we have non droppable b frame ?
        uint32_t bframeDelay;
        if(true==video_body->getNonClosedGopDelay(ptsStart,&bframeDelay) && bframeDelay)
        {
            if(bframeDelay+2*frameIncrement>delta) // i.e. delta is wrong
            {
                bframeDelay+=2*(int)frameIncrement-(int)delta;
                ADM_info("Some B-frames are not droppable, increasing delay by %d us\n",bframeDelay);
                videoDelay+=bframeDelay;
            }else
            {
                ADM_info("Some B-frames are not droppable, but PTS delay is sufficient\n");
            }
        }else
        {
            videoDelay=0;
        }

        //videoDelay
        if(delta>ptsStart)
        {
            videoDelay+=delta-ptsStart;
            dtsStart=0;
            ADM_info("Dts is too early, delaying everything by %" PRIu64" us\n",videoDelay);
        }else
        {
            dtsStart=ptsStart-delta;
        }
        endTime+=videoDelay;
        // Now search the DTS associated with it...
    }
    eofMet=false;
    currentFrame=0;
    this->startTimeDts=dtsStart;
    this->startTimePts=ptsStart;
    this->endTimePts=endTime;
    rewindTime=ptsStart;
    rewind();
    
    ADM_info(" Fixating start time by %d\n",abs((int)(startTime-startTimeDts)));
    ADM_info(" Starting DTS=%" PRIu64", PTS=%" PRIu64" ms\n",startTimeDts/1000,startTimePts/1000);
}
/**

*/
bool      ADM_videoStreamCopy::rewind(void)
{
    return video_body->GoToIntraTime_noDecoding(rewindTime);
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
    if(in>=startTimeDts) return in-startTimeDts;
    ADM_warning("Negative time!\n");
    ADM_warning("Current time = %" PRIu64"\n",in);
    ADM_warning("start time = %" PRIu64"\n",startTimeDts);
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
bool  ADM_videoStreamCopy::getPacket(ADMBitstream *out)
{
    if(true==eofMet) return false;
again:
    image.data=out->data;
    if(false==video_body->getCompressedPicture(rewindTime,videoDelay,&image))
    {
            ADM_warning("Get packet failed\n");
            return false;
    }
    out->len=image.dataLength;
    ADM_assert(out->len<=out->bufferSize);
#if 0
    if(image.demuxerPts!=ADM_NO_PTS)
        if(image.demuxerPts<startTimePts)   
        {
            if(image.flags & AVI_B_FRAME) 
            {
                ADM_warning("Dropping orphean B frame (PTS=%"PRIu64" ms)\n",image.demuxerPts/1000);
                goto again;
            }
        }
#endif
    out->pts=rescaleTs(image.demuxerPts);
    out->dts=rescaleTs(image.demuxerDts);
    if(image.demuxerPts!=ADM_NO_PTS)
    {
          if(image.demuxerDts!=ADM_NO_PTS)
          {
            if(image.demuxerPts<image.demuxerDts)
              {
                ADM_warning("PTS<DTS : PTS=%" PRIu64" ms , DTS=%" PRIu64"ms\n",image.demuxerPts/1000,image.demuxerDts/1000);

              }

          }
        if(image.demuxerPts>endTimePts ) 
        {
            eofMet=true;
            return false;
        }   
    }
    out->flags=image.flags;
    currentFrame++;
    return true;
}
/**
    \fn getVideoDuration
*/
uint64_t        ADM_videoStreamCopy::getVideoDuration(void)
{
    //return video_body->getVideoDuration();
    return endTimePts-startTimeDts;
}

/**

*/
bool     ADM_videoStreamCopy::providePts(void)
{
    return true;
}

// EOF
