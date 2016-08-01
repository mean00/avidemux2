
/***************************************************************************
    Class to handle libva accelerated renderer
    
    copyright            : (C) 2013 by mean
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

#ifndef T_LIBVARENDER_H
#define T_LIBVARENDER_H
#include "ADM_coreLibVA.h"
/**
    \fn class libvaRender
*/
class libvaRender: public VideoRenderBase
{
      protected:
                            GUI_WindowInfo info;
                            ADM_vaSurface  *mySurface[2];
                        bool cleanup(void);
                        int  toggle;
      public:
                             libvaRender( void ) ;
                             ~libvaRender();
              virtual	bool init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom);
              virtual	bool stop(void);				
              virtual   bool displayImage(ADMImage *pic);
              virtual   bool changeZoom(renderZoom newzoom);
              virtual   bool refresh(void);
              virtual   bool usingUIRedraw(void) {return false;}; // we can redraw by ourself
              virtual   ADM_HW_IMAGE getPreferedImage(void ) {return ADM_HW_LIBVA;}
                        const char *getName() {return "LibVA";}
};
#endif




