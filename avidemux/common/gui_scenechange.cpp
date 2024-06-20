/***************************************************************************
                          gui_scenechange.cpp  -  description
                             -------------------

            Detect scene change

    
    copyright            : (C) 2002/2008 by mean
                               2021 szlldm
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

static float histogramGUISceneChange[64];

/**
    \fn scChUtilCalcHisto
    \brief Calculate histogram
*/
void  scChUtilCalcHisto(ADMImage *img, float * hist)
{
    uint32_t w,h;
    int strides[3];
    uint8_t * planes[3], *ptr1, *ptr2;
    img->getWidthHeight(&w,&h);
    img->GetPitches(strides);
    img->GetReadPlanes(planes);
    
    uint32_t tmpHist[64];
    for (int i=0;i<64;i++)
        tmpHist[i] = 0;
    
    w /= 2;
    h /= 2;
    int u,v;
    for (int y=0; y<h; y++)
    {
        ptr1 = planes[1] + y*strides[1];
        ptr2 = planes[2] + y*strides[2];
        for (int x=0; x<w; x++)
        {
            u = *ptr1++;
            v = *ptr2++;
            u /= 8;
            v /= 8;
            tmpHist[u]++;
            tmpHist[v+32]++;
        }
    }

    for (int i=0;i<64;i++)
    {
        hist[i] = tmpHist[i];
        hist[i] /= w;
        hist[i] /= h;
    }
}

/**
    \fn initSCeneChangeDetector
    \brief Init Scene Change Detector
*/
void initSCeneChangeDetector(ADMImage *img)
{
    scChUtilCalcHisto(img, histogramGUISceneChange);
}
/**
    \fn isSCeneChange
    \brief Scene Change Detector
*/
bool  isSCeneChange(ADMImage *img)
{
    float * hist = histogramGUISceneChange;
    float nextHist[64];
    scChUtilCalcHisto(img, nextHist);
    float diff = 0;
    for (int i=0; i<64; i++)
    {
        diff += fabs(hist[i] - nextHist[i]);
    }
    diff = sqrt(diff);
    
    //printf("diff=%f\n",diff);
    if (diff > 0.5)
        return true;
    
    // adapt histogram
    for (int i=0; i<64; i++)
    {
        hist[i] = nextHist[i];
    }
    
    return false;
}

/**
    \fn GUI_PrevSceneChange
*/
void GUI_PrevSceneChange(void)
{
    if (playing)
        return;
    if (! avifileinfo)
        return;

    admPreview::deferDisplay(true);
    ADMImage *rdr;
    
    admPreview::samePicture();
    rdr=admPreview::getBuffer();
    if(rdr->refType!=ADM_HW_NONE) // need to convert it to plain YV12
    {
        if(false==rdr->hwDownloadFromRef())
        {
            ADM_warning("Cannot convert hw image to yv12\n");
            admPreview::deferDisplay(false);
            return;
        }
    }
    initSCeneChangeDetector(rdr);

    uint64_t startTime=admPreview::getCurrentPts();
    DIA_processingBase *work=createProcessing(QT_TRANSLATE_NOOP("scenechange", "Searching scene change.."),startTime);

    bool scFound=false;
    while(1)
    {
        UI_purge();

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
        if(isSCeneChange(rdr))
        {
            scFound = true;
            break;
        }
        if(work->update(1,startTime-admPreview::getCurrentPts()))
            break;
    }
    delete work;
    if (!scFound)
        admPreview::seekToTime(startTime);
    admPreview::deferDisplay(false);
    admPreview::samePicture();
    GUI_setCurrentFrameAndTime();
    return;
}

/**
    \fn GUI_NextSceneChange
*/
void GUI_NextSceneChange(void)
{
    if (playing)
        return;
    if (! avifileinfo)
        return;

    admPreview::deferDisplay(true);
    ADMImage *rdr;
    
    admPreview::samePicture();
    rdr=admPreview::getBuffer();
    if(rdr->refType!=ADM_HW_NONE) // need to convert it to plain YV12
    {
        if(false==rdr->hwDownloadFromRef())
        {
            ADM_warning("Cannot convert hw image to yv12\n");
            admPreview::deferDisplay(false);
            return;
        }
    }
    initSCeneChangeDetector(rdr);

    uint64_t duration=video_body->getVideoDuration();    
    uint64_t startTime=admPreview::getCurrentPts();
    DIA_processingBase *work=createProcessing(QT_TRANSLATE_NOOP("scenechange", "Searching scene change.."),duration-startTime);

    bool scFound=false;
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
        if(isSCeneChange(rdr))
        {
            scFound = true;
            break;
        }
        if(work->update(1,admPreview::getCurrentPts()-startTime))
            break;
    }
    delete work;
    if (!scFound)
        admPreview::seekToTime(startTime);
    admPreview::deferDisplay(false);
    admPreview::samePicture();
    GUI_setCurrentFrameAndTime();
    return;
}

//EOF
