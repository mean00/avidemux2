/***************************************************************************
                          \fn ADM_videoFilterBridge
                          \brief Interface between editor & filter chain
                           (c) Mean 2009



 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_videoFilterBridge.h"
#include "ADM_editor/ADM_edit.hxx"
extern ADM_Composer *video_body;
/**
    \fn ADM_videoFilterBridge

*/
ADM_videoFilterBridge::ADM_videoFilterBridge(uint64_t startTime, uint64_t endTime) : ADM_coreVideoFilter(NULL,NULL)
{
    printf("[VideoFilterBridge] Creating bridge from %"LU" s to %"LU" s\n",(uint32_t)(startTime/1000000LL),(uint32_t)(endTime/1000000LL));
    this->startTime=startTime;
    if(endTime==-1LL)
    {
        uint64_t total=video_body->getVideoDuration();
        endTime=total-startTime+1;
    }
    this->endTime=endTime;
    myName="Bridge";
    aviInfo fo;
    video_body->getVideoInfo(&fo);
    bridgeInfo.width=fo.width;
    bridgeInfo.height=fo.height;
    bridgeInfo.frameIncrement=video_body->getFrameIncrement();
    bridgeInfo.totalDuration=endTime-startTime;
    bridgeInfo.frameIncrement=video_body->getFrameIncrement();
    rewind();
}
/**
    \fn rewind
    \brief go or return to the original position...
*/
bool ADM_videoFilterBridge::rewind(void)
{
    return goToTime(0);
}

/**
    \fn ADM_videoFilterBridge

*/
ADM_videoFilterBridge::~ADM_videoFilterBridge()
{

}
/**
    \fn getNextFrame
    \brief
*/
bool         ADM_videoFilterBridge::getNextFrame(uint32_t *frameNumber,ADMImage *image)
{
again:
    bool r=false;
    if(firstImage==true)
    {
        firstImage=false;
        r=video_body->samePicture(image);
        lastSentImage=0;
        *frameNumber=nextFrame=0;
    }else
    {
        r=   video_body->nextPicture(image);
        nextFrame++;
        *frameNumber=nextFrame;
        lastSentImage++;
    }
    if(r==false) return false;
    // Translate pts if any
    int64_t pts=image->Pts;
    if(pts>endTime)
    {
        printf("[VideoBridge] This frame is too late (%"LLD" vs %"LLU")\n",pts,endTime);
        return false;
    }
    if(pts<startTime) 
    {
            printf("[VideoBridge] This frame is too early (%"LLD" vs %"LLU")\n",pts,startTime);
            goto again;
    }
    // Rescale time
    image->Pts-=startTime;
    return true;
}
/**
    \fn ADM_videoFilterBridge

*/
FilterInfo  *ADM_videoFilterBridge::getInfo(void)
{
    return &bridgeInfo;
}
/**
    \fn goToTime
*/
bool         ADM_videoFilterBridge::goToTime(uint64_t usSeek)
{
    if(!usSeek)
        video_body->goToTimeVideo(startTime+usSeek);
    else
    {
        uint64_t seek=usSeek;
        if(true==video_body->getPKFramePTS(&seek))
        {
                video_body->goToIntraTimeVideo(seek);
        }else ADM_warning("Cannot find previous keyframe\n");
    }
    firstImage=true;
    lastSentImage=0;
    return true;
}
// EOF
