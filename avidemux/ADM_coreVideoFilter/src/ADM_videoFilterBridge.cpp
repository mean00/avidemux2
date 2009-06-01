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

#include "ADM_default.h"
#include "ADM_videoFilterBridge.h"
#include "ADM_editor/ADM_edit.hxx"
extern ADM_Composer *video_body;
/**
    \fn ADM_videoFilterBridge

*/
ADM_videoFilterBridge::ADM_videoFilterBridge(uint64_t startTime, uint64_t endTime) : ADM_coreVideoFilter(NULL,NULL)
{
    this->startTime=startTime;
    this->endTime=endTime;
    
    aviInfo fo;
    video_body->getVideoInfo(&fo);
    bridgeInfo.width=fo.width;
    bridgeInfo.height=fo.height;
    bridgeInfo.frameIncrement=video_body->getFrameIncrement();
    bridgeInfo.totalDuration=video_body->getVideoDuration();
    bridgeInfo.frameIncrement=video_body->getFrameIncrement();
    rewind();
}
/**
    \fn rewind
    \brief go or return to the original position...
*/
bool ADM_videoFilterBridge::rewind(void)
{
    video_body->GoToTime(startTime);
    return true;
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
bool         ADM_videoFilterBridge::getNextFrame(ADMImage *image)
{
    return   video_body->NextPicture(image);
}
/**
    \fn ADM_videoFilterBridge

*/
FilterInfo  *ADM_videoFilterBridge::getInfo(void)
{
    return &bridgeInfo;
}
// EOF
