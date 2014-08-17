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
    uint32_t width = img->_width;
    int       stride=img->GetPitch(PLANAR_Y);
    uint32_t height = img->_height;
    uint32_t sliceOffset = (stride * height)>>3 ;    // 1/8 of an image

    uint8_t *buff,*start;

    int cnt4=0;

    start=img->GetReadPtr(PLANAR_Y)+ sliceOffset*sliceNum;
    buff=start+sliceOffset;

    while(--buff>start)
    {
      if(*buff > darkness )
      {
        cnt4++;
        if(cnt4>=maxnonb)
          return(1);
      }
    }
    return(0);

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
    GUI_Error_HIG("BlackFrame","This function is unsupported at the moment");
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
    DIA_processingBase *work=createProcessing(QT_TR_NOOP("Searching black frame.."));

    uint64_t startTime=admPreview::getCurrentPts();
    uint64_t totalTime=video_body->getVideoDuration()-startTime;
    
    double percent;
    Clock refresh;

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
                break;
        }
        // not black..
        // Only refresh every 3 sec
        if(refresh.getElapsedMS()>3000) 
        {
            if(totalTime>1)
            {
                    uint64_t curTime=admPreview::getCurrentPts();
                    percent=(curTime-startTime)/(double)totalTime;
                    percent*=1000;
                    if(work->update((int)percent,1000))         
                          break;
                    if(!work->isAlive())
                          break;
            }
            else
            {
                    percent=500;
            }
            
            GUI_setCurrentFrameAndTime();
            refresh.reset();
        }
    }
    delete work;
    admPreview::deferDisplay(false);
    admPreview::samePicture();
    GUI_setCurrentFrameAndTime();
    return;

}

//EOF
