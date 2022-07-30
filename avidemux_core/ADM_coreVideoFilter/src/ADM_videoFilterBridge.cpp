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
#include "ADM_vidMisc.h"

/**
    \fn ADM_videoFilterBridge

*/
ADM_videoFilterBridge::ADM_videoFilterBridge(IEditor *editor, uint64_t startTime, uint64_t endTime) : ADM_coreVideoFilter(NULL, NULL)
{
    ADM_info("Creating instance at %p\n",this);
    ADM_assert(editor);

    this->editor = editor;
    myName = "Bridge";
    updateBridge(startTime, endTime);
    rewind();
}

/**
    \fn     updateBridge
    \brief
*/
void ADM_videoFilterBridge::updateBridge(uint64_t startTime, uint64_t endTime)
{
    if (endTime == ADM_NO_PTS)
    {
        endTime = editor->getVideoDuration();
        if(endTime < startTime) startTime = endTime;
    }
    this->startTime = startTime;
    this->endTime = endTime;

    char *str = ADM_strdup(ADM_us2plain(this->startTime));
    ADM_info("Using time range from %s to %s\n",str,ADM_us2plain(this->endTime));
    ADM_dealloc(str);
    str = NULL;

    aviInfo fo;
    editor->getVideoInfo(&fo);
    bridgeInfo.width = fo.width;
    bridgeInfo.height = fo.height;
    bridgeInfo.frameIncrement = editor->getFrameIncrement();
    editor->getTimeBase(&(bridgeInfo.timeBaseNum), &(bridgeInfo.timeBaseDen));
    bridgeInfo.totalDuration = endTime - startTime;
    bridgeInfo.markerA = editor->getMarkerAPts();
    bridgeInfo.markerB = editor->getMarkerBPts();
}

/**
    \fn     getNextFrameBase
    \brief
*/
bool         ADM_videoFilterBridge::getNextFrameBase(uint32_t *frameNumber, ADMImage *image)
{
again:
    bool r = false;

    if (firstImage == true)
    {
        firstImage = false;
        r = editor->samePicture(image);
        lastSentImage = 0;
        *frameNumber = nextFrame = 0;
    }
    else
    {
        r =   editor->nextPicture(image);
        nextFrame++;
        *frameNumber = nextFrame;
        lastSentImage++;
    }

    if (r == false)
    {
        return false;
    }

    // Translate pts if any
    int64_t pts = image->Pts;

    if (pts >= endTime)
    {
        ADM_warning("[VideoBridge] This frame is too late (%" PRId64" vs %" PRIu64")\n", pts, endTime);
        return false;
    }

    if (pts < startTime)
    {
        ADM_warning("[VideoBridge] This frame is too early (%" PRId64" vs %" PRIu64")\n", pts, startTime);
        goto again;
    }

    // Rescale time
    image->Pts -= startTime;
    return true;
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
    printf("[VideoFilterBridge] Destroying instance at %p\n",this);
}
/**
    \fn getNextFrame
    \brief
*/
bool         ADM_videoFilterBridge::getNextFrame(uint32_t *frameNumber, ADMImage *image)
{
    return getNextFrameAs(ADM_HW_NONE, frameNumber, image);
}

/**
    \fn getNextFrameAs
    \brief
*/
bool         ADM_videoFilterBridge::getNextFrameAs(ADM_HW_IMAGE type, uint32_t *frameNumber, ADMImage *image)
{
    if (false == getNextFrameBase(frameNumber, image))
    {
        ADM_warning("[Bridge] Base did not get an image\n");
        return false;
    }

    // Check if image is
    if (ADM_HW_ANY == type)
    {
        return true;
    }

    if (type != image->refType)
    {
        return image->hwDownloadFromRef(); // nope, revert to base type
    }

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
bool         ADM_videoFilterBridge::goToTime(uint64_t usSeek, bool fineSeek)
{
    if (!usSeek)
    {
        editor->goToTimeVideo(startTime);
    }
    else
    {
        usSeek += startTime;
        if (fineSeek)
        {
            // When fine-seeking to a time very close to marker positions,
            // substitute it with the closest marker as filters which modify
            // timing may introduce loss of precision. Marker A has priority.
#define MAGNETIC_RANGE 100
            uint64_t markerDiff = (usSeek > bridgeInfo.markerA)? usSeek - bridgeInfo.markerA : bridgeInfo.markerA - usSeek;
            if (markerDiff < MAGNETIC_RANGE)
            {
                usSeek = bridgeInfo.markerA;
            } else
            {
                markerDiff = (usSeek > bridgeInfo.markerB)? usSeek - bridgeInfo.markerB : bridgeInfo.markerB - usSeek;
                if (markerDiff < MAGNETIC_RANGE)
                    usSeek = bridgeInfo.markerB;
            }
            if (false == editor->goToTimeVideo(usSeek))
                fineSeek = false;
        }
        if (!fineSeek)
        {
            usSeek++; // avoid skipping to the previous keyframe on repeated calls
            // don't fail if target time matches marker B at default position, i.e. the full duration of the video
            uint64_t maxDur = editor->getVideoDuration();
            if (maxDur && usSeek >= maxDur)
                usSeek = maxDur - 1;
            if (editor->getPKFramePTS(&usSeek))
            {
                editor->goToIntraTimeVideo(usSeek);
            }
            else
            {
                ADM_warning("Cannot find previous keyframe\n");
            }
        }
    }

    firstImage = true;
    lastSentImage = 0;
    return true;
}
// EOF
