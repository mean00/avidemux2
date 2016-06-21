/**
    \author mean fixounet@free.fr 2010
*/

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
#include "DIA_coreToolkit.h"
#include "GUI_render.h"
#include "GUI_renderInternal.h"
#include "GUI_accelRender.h"

class simpleRender: public VideoRenderBase
{
      protected:
      public:
                             simpleRender( void ) ;
                             ~simpleRender();
              virtual	bool init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom);
              virtual	bool stop(void);				
              virtual   bool displayImage(ADMImage *pic);
              virtual   bool changeZoom(renderZoom newZoom);
              virtual   bool refresh(void);
              virtual   bool usingUIRedraw(void) {return false;};
                  const char *getName() {return "Dummy";}
};



VideoRenderBase *spawnSimpleRender()
{
    return new simpleRender();
}


/**
    \fn simpleRender
*/
simpleRender::simpleRender()
{
    ADM_info("creating dummy render.\n");
}
/**
    \fn simpleRender
*/
simpleRender::~simpleRender()
{
    ADM_info("Destroying dummy render.\n");
}

/**
    \fn stop
*/
bool simpleRender::stop(void)
{
    ADM_info("stopping dummy render.\n");
    return true;
}
/**
    \fn refresh
*/
bool simpleRender::refresh(void)
{
     return true;
}
/**
    \fn displayImage
*/
bool simpleRender::displayImage(ADMImage *pic)
{
        return true;
}
/**
    \fn changeZoom
*/
bool simpleRender::changeZoom(renderZoom newZoom)
{
        return true;
}
/**
    \fn changeZoom
*/
bool simpleRender::init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom)
{
    ADM_info("init, simple render. w=%d, h=%d,zoom=%d\n",(int)w,(int)h,(int)zoom);
    return true;
}

