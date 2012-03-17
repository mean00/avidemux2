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
#include "ADM_cpp.h"

#include "ADM_default.h"
#include "ADM_edit.hxx"
#include "ADM_render/GUI_render.h"
#include "ADM_commonUI/GUI_ui.h"
#include "ADM_preview.h"
#include "ADM_imageResizer.h"
#include "DIA_coreToolkit.h"

extern uint8_t              UI_getPhysicalScreenSize(void* window, uint32_t *w,uint32_t *h);
static void                 previewBlit(ADMImage *from,ADMImage *to,uint32_t startx,uint32_t starty);
static ADM_PREVIEW_MODE     previewMode=ADM_PREVIEW_NONE;

static bool                 defered_display=false;  /* When 1, it means we are in playback mode */
static renderZoom           zoom=ZOOM_1_1;
static ADMImage             *resized=NULL;
static ADMImageResizer      *resizer=NULL;


/*************************************/
ADMImage *admPreview::rdrImage=NULL; /* Unprocessed image */

static uint32_t  rdrPhysicalW=0;  /* W*H of the INPUT window */
static uint32_t  rdrPhysicalH=0;



/*************************************/
extern ADM_Composer *video_body;
/**
    \fn getBuffer
*/
ADMImage *admPreview::getBuffer(void)
{
    return rdrImage;
}
/**
    \fn getPreferedHwImageFormat
*/
ADM_HW_IMAGE admPreview::getPreferedHwImageFormat(void)
{
    return renderGetPreferedImageFormat();
}
/**
      \fn admPreview::setMainDimension
      \brief Update width & height of input video
      @param w : width
      @param h : height
*/

void admPreview::setMainDimension(uint32_t w, uint32_t h,renderZoom nzoom)
{
  
  if(rdrImage) delete rdrImage;
  rdrImage=new ADMImageDefault(w,h);
  rdrPhysicalW=w;
  rdrPhysicalH=h;
  if(nzoom==ZOOM_AUTO)
  {
      uint32_t phyW,phyH;
      UI_getPhysicalScreenSize(NULL, &phyW,&phyH);
      if(3*phyW<4*w || 3*phyH<4*h)
      {
          if(phyW<w/2 || phyH<h/2)
          {
                    nzoom=ZOOM_1_4;
          }else
          {
                    nzoom=ZOOM_1_2;
          }
         
      }
      else
      {
          nzoom=ZOOM_1_1;
      }
  }
  zoom=nzoom;
  renderDisplayResize(rdrPhysicalW,rdrPhysicalH,zoom);
 // Install our hook, we will do it more than needed
 // but it does not really harm
  renderHookRefreshRequest(admPreview::updateImage);
}
/**
    \fn getCurrentZoom
*/
renderZoom admPreview::getCurrentZoom()
{
    return zoom;
}

/**
      \fn getPreviewMode
      \brief returns current preview mode
      @return current preview mode

*/

ADM_PREVIEW_MODE getPreviewMode(void)
{
  return previewMode; 
}
/**
    \fn changePreviewZoom
*/
void changePreviewZoom(renderZoom nzoom)
{
    admPreview::stop();
    ADM_info("Preview :: Change zoom %d->%d\n",zoom,nzoom);
    zoom=nzoom;
    renderDisplayResize(rdrPhysicalW,rdrPhysicalH,zoom);
    admPreview::start();
}

/**
      \fn setPreviewMode
      \brief set current preview mode an update UI
      @param mode  new preview mode

*/

 void setPreviewMode(ADM_PREVIEW_MODE mode)
{
  previewMode=mode; 
  UI_setCurrentPreview( (int)mode);
}
/**
      \fn deferDisplay
      \brief Enable or disable defered display. Used by playback mode to have smooth output

*/
void admPreview::deferDisplay(bool onoff)
{
      
      if(true==onoff)
      {
        renderStartPlaying();
        defered_display=1;
      }
      else
      {
        renderStopPlaying();
        defered_display=0;
      }
}
/**
      \fn admPreview::start
      \brief start preview, reiginite preview with new parameters

*/

void 	admPreview::start( void )
{
            ADM_info("admPreview,starting\n");
}
/**
      \fn admPreview::stop
      \brief kill preview  and associated datas
*/

void admPreview::stop( void )
{
      renderLock();
     
      renderUnlock();
}

/**
      \fn previewBlit(ADMImage *from,ADMImage *to,uint32_t startx,uint32_t starty)
      \brief Blit "from" to "to" at position startx,starty
*/

void previewBlit(ADMImage *from,ADMImage *to,uint32_t startx,uint32_t starty)
{
  
  from->copyTo(to,startx,starty);
  
}
/**
      \fn     displayNow
      \brief  display on screen immediately. The image has been calculated previously by update
*/

void admPreview::displayNow(void)
{

    uint32_t fl,len;	
    
    switch(previewMode)
    {
      case ADM_PREVIEW_NONE:
      case ADM_PREVIEW_OUTPUT:     
        renderUpdateImage(rdrImage);
        break;
      default: ADM_assert(0);break;
    }
}
/**
    \fn cleanUp
    \brief do the cleanup, what else ?
*/
void admPreview::cleanUp(void)
{
	admPreview::stop();

	destroy();
}
/**
    \fn cleanUp
    \brief do the cleanup, what else ?
*/
void admPreview::destroy(void)
{
    ADM_info("Destroying preview\n");
	if(rdrImage)
	{
		delete rdrImage;
		rdrImage = NULL;
	}
}
/**
    \fn updateImage
*/
bool admPreview::updateImage(void)
{
    UI_setFrameType(  rdrImage->flags,rdrImage->_Qp);
    if(!defered_display) 
        renderUpdateImage(rdrImage);                  
    return true;
}
// EOF
