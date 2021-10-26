/***************************************************************************
                          gui_blackframes.cpp  -  description
                             -------------------

            Detect black frames

    
    copyright            : (C) 2002/2008 by mean
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

#include "ADM_cpp.h"
#include "avi_vars.h"

#include <math.h>

#include "DIA_fileSel.h"
#include "ADM_assert.h"
#include "prototype.h"
#include "audio_out.h"
#include "ADM_coreAudio.h"
#include "gui_action.hxx"
#include "gtkgui.h"
#include "DIA_coreToolkit.h"
#include "ADM_render/GUI_render.h"
#include "DIA_working.h"
#include "DIA_processing.h"
#include "ADM_commonUI/DIA_busy.h"
#include "ADM_commonUI/GUI_ui.h"

#include "ADM_vidMisc.h"
#include "ADM_preview.h"


static const int  sliceOrder[8]={3,4,2,5,1,6,0,7};
/**
    \fn sliceScanNotBlack
    \brief The image is split into 8 slices, returns if the given slice is black or not
*/
static int sliceScanNotBlack(int darkness, int maxnonb, int sliceNum,ADMImage *img)
{
    uint32_t height = img->_height;
    uint32_t width = img->_width;
    int      stride = img->GetPitch(PLANAR_Y);
    uint8_t *buff = img->GetReadPtr(PLANAR_Y)+ stride*(height>>3)*sliceNum;    // 1/8 of an image
    int cnt4=0;

    for (int y=0; y<(height>>3); y++)
    {
        for (int x=0; x<width; x++)
        {
            if(buff[x] > darkness)
            {
                cnt4++;
                if(cnt4>=maxnonb)
                    return 1;
            }
        }
        buff += stride;
    }

    return 0;
}
/**
    \fn fastIsNotBlack
    \brief Quickly check if the frame is black or not
*/
uint8_t  fastIsNotBlack(int darkness,ADMImage *img)
{

    uint32_t width = img->_width;
    uint32_t height = img->_height;    
    uint32_t maxnonb=(width* height)>>8;

    maxnonb>>=3;
    // Divide the screen in 8 part  : 0 1 2 3 4 5 6 7 
    // Scan 2 & 3 first, if still good, go on
    for(uint32_t i=0;i<6;i++)
    {
        if(sliceScanNotBlack(darkness,maxnonb,sliceOrder[i],img)) return 1;
    }
    // The slice 0 & 7 are particular and we admit twice as much
    if(sliceScanNotBlack(darkness,maxnonb*2,0,img)) return 1;
    if(sliceScanNotBlack(darkness,maxnonb*2,7,img)) return 1;

    return(0);
}
/**
    \fn GUI_PrevBlackFrame
    \brief lookup for a black frame
*/
void GUI_PrevBlackFrame(void)
{
    if (playing)
        return;
    if (! avifileinfo)
        return;
    const int darkness=40;
    admPreview::deferDisplay(true);
    ADMImage *rdr;

    uint64_t startTime=admPreview::getCurrentPts();
    uint64_t lastBlackPts=ADM_NO_PTS;
    DIA_processingBase *work=createProcessing(QT_TRANSLATE_NOOP("blackframes", "Searching black frame.."),startTime);
    
    // search among (likely) cached images
    for (int i=0; i<6; i++)
    {
        if(false==admPreview::previousPicture())
            break;
        rdr=admPreview::getBuffer();
        if(rdr->refType!=ADM_HW_NONE) // need to convert it to plain YV12
        {
            if(false==rdr->hwDownloadFromRef())
            {
                ADM_warning("Cannot convert hw image to yv12\n");
                break;
            }
        }
        if(!fastIsNotBlack(darkness,rdr))
        {
            lastBlackPts = admPreview::getCurrentPts();
            break;
        }
    }
    
    if (lastBlackPts==ADM_NO_PTS)
    {
        uint64_t anchorPts = startTime;
        admPreview::seekToTime(anchorPts);
        uint64_t gopPts;
        bool firstBlock=false;
        bool error=false;
        while(1)
        {
            UI_purge();
            if (firstBlock)
                break;
            if (false==admPreview::previousKeyFrame())
            {
                video_body->rewind();
                admPreview::samePicture();
                firstBlock = true;
            }
            gopPts = admPreview::getCurrentPts();
            while(1)
            {
                UI_purge();
                if (admPreview::getCurrentPts() >= anchorPts)
                {
                    if (!firstBlock)
                    {
                        if (false==admPreview::previousKeyFrame())
                            error = true;
                        anchorPts = admPreview::getCurrentPts();
                    }
                    break;              
                }
                rdr=admPreview::getBuffer();
                if(rdr->refType!=ADM_HW_NONE) // need to convert it to plain YV12
                {
                    if(false==rdr->hwDownloadFromRef())
                    {
                        ADM_warning("Cannot convert hw image to yv12\n");
                        break;
                    }
                }
                if(!fastIsNotBlack(darkness,rdr))
                {
                    lastBlackPts = admPreview::getCurrentPts();
                }
                if(work->update(1,startTime-anchorPts + admPreview::getCurrentPts()-gopPts))
                {
                    error = true;
                    break;
                }
                if(false==admPreview::nextPicture())
                {
                    error = true;
                    break;
                }
            }
            if (error)
            {
                lastBlackPts=ADM_NO_PTS;
                break;
            }
            if (lastBlackPts!=ADM_NO_PTS)
                break;
        }
    }
    delete work;
    admPreview::seekToTime((lastBlackPts==ADM_NO_PTS)?startTime:lastBlackPts);
    admPreview::deferDisplay(false);
    admPreview::samePicture();
    GUI_setCurrentFrameAndTime();
    return;
}

/**
    \fn GUI_NextBlackFrame
    \brief lookup for a black frame
*/
void GUI_NextBlackFrame(void)
{
    if (playing)
        return;
    if (! avifileinfo)
        return;
    const int darkness=40;
    admPreview::deferDisplay(true);
    ADMImage *rdr;

    uint64_t duration=video_body->getVideoDuration();    
    uint64_t startTime=admPreview::getCurrentPts();
    DIA_processingBase *work=createProcessing(QT_TRANSLATE_NOOP("blackframes", "Searching black frame.."),duration-startTime);

    bool blackFound=false;
    while(1)
    {
        UI_purge();

        if(false==admPreview::nextPicture())
            break;
        rdr=admPreview::getBuffer();
        if(rdr->refType!=ADM_HW_NONE) // need to convert it to plain YV12
        {
            if(false==rdr->hwDownloadFromRef())
            {
                ADM_warning("Cannot convert hw image to yv12\n");
                break;
            }
        }
        if(!fastIsNotBlack(darkness,rdr))
        {
            blackFound = true;
            break;
        }
        if(work->update(1,admPreview::getCurrentPts()-startTime))
            break;
    }
    delete work;
    if (!blackFound)
        admPreview::seekToTime(startTime);
    admPreview::deferDisplay(false);
    admPreview::samePicture();
    GUI_setCurrentFrameAndTime();
    return;
}

//EOF
