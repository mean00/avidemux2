/***************************************************************************
    
    Handle preview mode
    
    It is displayed in 3 layers
    
    
    Engine : Call setPreviewMode and amdPreview depending on the actions
            previewMode is the **current** preview mode
    
    admPreview : 
          Allocate/desallocate ressources
          Build preview window
          Call display properly to display it
          
    GUI_PreviewXXXX
          UI toolkit to actually display the window
          See GUI_ui.h to see them
                 
    
    
    copyright            : (C) 2007 by mean
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
#include "config.h"

#include "ADM_default.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_render/GUI_render.h"

#include "ADM_debugID.h"
#include "ADM_commonUI/GUI_ui.h"
#include "ADM_preview.h"

#include "DIA_coreToolkit.h"

#define MAX(a,b) ( (a)>(b) ? (a) : (b) )

/*************************************/
/*************************************/
extern ADM_Composer *video_body;
/**
    \fn getCurrentPts
    \brief returns the PTS in us of the last displayed frame
*/
uint64_t admPreview::getCurrentPts(void)
{
        if(rdrImage) return rdrImage->Pts;
}
/**
      \fn admPreview::seekToTime
      \brief Seek to any given frame
      
      @param timeframe Time of the image 
*/

bool admPreview::seekToTime(uint64_t timeframe)
{
    if(!video_body->goToTimeVideo(timeframe)) 
    {
        ADM_warning(" seeking for frame at %"LLU" ms failed\n",timeframe/1000LL);
        return false;
    }
    return samePicture();
}
/**
      \fn admPreview::seekToIntraPts
      \brief Seek to intra at PTS given as arg
      
      @param timeframe Time of the image 
*/

bool admPreview::seekToIntraPts(uint64_t timeframe)
{
    if(!video_body->goToIntraTimeVideo(timeframe)) 
    {
        ADM_warning(" seeking for frame at %"LLU" ms failed\n",timeframe/1000LL);
        return false;
    }
    return samePicture();
}
/**
    \fn samePicture
*/
uint8_t admPreview::samePicture(void)
{
    if(!video_body->samePicture(rdrImage)) return false;
    return updateImage();
}
/**
      \fn admPreview::update
      \brief display data associated with framenum image
      @param image : current main image (input)
      @param framenum, framenumber
*/

uint8_t admPreview::nextPicture(void)
{
   
   if(!video_body->nextPicture(rdrImage)) return 0;
   return updateImage();
}

/**
      \fn admPreview::update
      \brief display data associated with framenum image
      @param image : current main image (input)
      @param framenum, framenumber
*/

uint8_t admPreview::previousPicture(void)
{
    if(!video_body->previousPicture(rdrImage)) return 0;
    return updateImage();
}
/**
    \fn nextKeyFrame

*/
bool admPreview::nextKeyFrame(void)
{
    uint64_t pts=getCurrentPts();
    ADM_info("Current PTS :%"LLD" ms\n",pts/1000LL);
    if(false==video_body->getNKFramePTS(&pts))
    {
        ADM_warning("Cannot find next keyframe\n");
        return false;
    }
    ADM_info("next kf PTS :%"LLD" ms\n",pts/1000LL);
    return seekToIntraPts(pts);
}
/**
    \fn previousKeyFrame

*/
bool admPreview::previousKeyFrame(void)
{
    uint64_t pts=getCurrentPts();
    ADM_info("Current PTS :%"LLD" ms\n",pts/1000LL);
    if(false==video_body->getPKFramePTS(&pts))
    {
        ADM_warning("Cannot find previous keyframe\n");
        return false;
    }
    ADM_info("next kf PTS :%"LLD" ms\n",pts/1000LL);
    return seekToIntraPts(pts);
}
// EOF
