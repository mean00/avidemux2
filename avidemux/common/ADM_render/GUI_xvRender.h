
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
#pragma once
/**
    \fn class XvRender
*/
class XvRender: public VideoRenderBase
{
      protected:
                        unsigned int xv_port;
                        uint32_t xv_format;
                        Display *xv_display;
                        XvImage *xvimage;
                        GC xv_gc;
                        XGCValues xv_xgc;
                        Window xv_win;

      protected:
                             GUI_WindowInfo info;
                        bool    lowLevelXvInit( GUI_WindowInfo *  window, uint32_t w, uint32_t h);
                        bool    xvDraw(uint32_t w,uint32_t h,uint32_t destW,uint32_t destH);
                        bool    lookupYV12(Display * dis, uint32_t port, uint32_t * fmt);
                        Atom    getAtom(const char *string,Display *xv_display,unsigned int xv_port);
                        void    displayAdaptorInfo(int num,XvAdaptorInfo *curai);

                        
                        
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





