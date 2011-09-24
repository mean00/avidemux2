//
// C++ Interface: GUI_accelRender
//
// Description: 
//  Class to hold all accelerated display (xv, sdl....)
//
// Author: mean <fixounet@free.fr>, (C) 2004/2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GUI_ACCELRENDER_H
#define GUI_ACCELRENDER_H
#include "ADM_colorspace.h"
/**
    \class VideoRenderBase
*/
class VideoRenderBase
{
      protected:
                        ADMColorScalerFull *scaler;
                        uint32_t imageWidth;
                        uint32_t imageHeight;
                        uint32_t displayWidth;
                        uint32_t displayHeight;
                        renderZoom currentZoom;

                        bool calcDisplayFromZoom(renderZoom zoom);
                        bool baseInit(uint32_t w,uint32_t h,renderZoom zoom);
       public:
                                VideoRenderBase( void) {scaler=NULL;currentZoom=ZOOM_1_1;};
              virtual          ~VideoRenderBase() {if(scaler) delete scaler;scaler=NULL;}
              virtual	bool    init(GUI_WindowInfo * window, uint32_t w, uint32_t h,renderZoom zoom)=0;
              virtual	bool    stop(void)=0;
              virtual   bool    displayImage(ADMImage *pic)=0;
              virtual   bool    refresh(void) {return true;}
              virtual   bool    changeZoom(renderZoom newzoom)=0;
              virtual   bool    usingUIRedraw(void)=0;
              virtual   ADM_HW_IMAGE getPreferedImage(void ) {return ADM_HW_NONE;}
};
#endif
