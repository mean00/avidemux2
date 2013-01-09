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
    ADM_info("Creating copy video stream, start time=%2.2f s\n",(float)startTime/1000000.);
    video_body->getVideoInfo(&info);
    width=info.width;
    height=info.height;
    fourCC=info.fcc;
    averageFps1000=info.fps1000;
    frameIncrement=video_body->getFrameIncrement();
    isCFR=false;
    // Estimate start frame
    if(false==video_body->getPKFramePTS(&ptsStart))
    {
        ADM_warning("Cannot find previous keyframe\n");
        ptsStart=dtsStart=startTime;
    }else   
    {
        uint64_t delta=ptsStart;
        video_body->getPtsDtsDelta(&delta);

// The next DTS must be used if the first one is 0 and the second one is more than a frame later (DTS can be e.g. 0,800,820...)
        _SEGMENT *seg0=video_body->getSegment(0);
        if (startTime<seg0->_startTimeUs+seg0->_durationUs)
        {
        uint64_t seg0Pts=seg0->_refStartTimeUs,pts,dts,dts2;
        uint32_t flags;

        if (video_body->getVideoPtsDts(1,&flags,&pts,&dts2) && dts2>frameIncrement && dts2!=ADM_NO_PTS
        && video_body->getVideoPtsDts(0,&flags,&pts,&dts) && dts==0
        && (seg0Pts==0 || pts==seg0Pts) && (startTime==0 || startTime==pts))
         {
             delta=pts-(dts2-frameIncrement);
         }
        }

        ADM_info("PTS/DTS delta=%"PRIu64" us\n",delta);
        //videoDelay
        if(delta>ptsStart)
        {
            videoDelay=delta-ptsStart;
            dtsStart=0;
            ADM_info("Dts is too early, delaying everything by %"PRIu64" ms\n",videoDelay/1000);
        }else
        {
            dtsStart=ptsStart-delta;
        }
        // Now search the DTS associated with it...
    }
    eofMet=false;

    this->startTimeDts=dtsStart;
    this->startTimePts=ptsStart;
    this->endTimePts=endTime;
    rewindTime=ptsStart;
    rewind();
    
    ADM_info(" Fixating start time by %d\n",abs((int)(startTime-startTimeDts)));
    ADM_info(" Starting DTS=%"PRIu64", PTS=%"PRIu64" ms\n",startTimeDts/1000,startTimePts/1000);
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
    ADM_warning("Current time = %"PRIu64"\n",in);
    ADM_warning("start time = %"PRIu64"\n",startTimeDts);
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
    if(false==video_body->getCompressedPicture(videoDelay,&image))
    {
            ADM_warning(" Get packet failed ");
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
                ADM_warning("PTS<DTS : PTS=%"PRIu64" ms , DTS=%"PRIu64"ms\n",image.demuxerPts/1000,image.demuxerDts/1000);

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
    return endTimePts-startTimePts;
}

/**

*/
bool     ADM_videoStreamCopy::providePts(void)
{
    return true;
}

// EOF
