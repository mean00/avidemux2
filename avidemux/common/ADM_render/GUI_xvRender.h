
/***************************************************************************
    Class to handle Xv accelerated renderer
    
    copyright            : (C) 2006 by mean
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

#ifndef T_XVRENDER_H
#define T_XVRENDER_H
/**
    \fn class XvRender
*/
class XvRender: public VideoRenderBase
{
      protected:
                            GUI_WindowInfo info;
      public:
                             XvRender( void ) ;
              virtual        ~XvRender();
              virtual	bool init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom);
              virtual	bool stop(void);				
              virtual   bool displayImage(ADMImage *pic);
              virtual   bool changeZoom(renderZoom newzoom);
              virtual   bool refresh(void);
              virtual   bool usingUIRedraw(void) {return false;}; // we can redraw by ourselves
                        const char *getName() {return "XVideo";}
};
#endif




