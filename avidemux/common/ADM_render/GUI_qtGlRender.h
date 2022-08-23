/***************************************************************************
    copyright            : (C) 2010 by gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef GUI_QTGLRENDER_H
#define GUI_QTGLRENDER_H

#include "GUI_render.h"
#include "GUI_accelRender.h"
#include "ADM_colorspace.h"
#include "ADM_openGl.h"

/**
    \fn class QtGlRender
*/
class QtGlRender: public VideoRenderBase
{
      protected:

                            GUI_WindowInfo  info;
                            QtGlAccelWidget *glWidget;
      public:
                             QtGlRender( void ) ;
              virtual        ~QtGlRender();
              virtual	bool init( GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom);
              virtual	bool stop(void);
              virtual   bool displayImage(ADMImage *pic);
              virtual   bool changeZoom(float newzoom);
              virtual   bool refresh(void);
              virtual   bool usingUIRedraw(void) {return false;}; // We can! redraw by ourself

                        const char *getName() {return "QtGl";}
};

#endif
