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
#include "GUI_simpleRender.h"

extern void MUI_rgbDraw(void *widg,uint32_t w, uint32_t h,uint8_t *ptr);
extern void *MUI_getDrawWidget(void);
/**
    \fn simpleRender
*/
simpleRender::simpleRender()
{
    ADM_info("creating simple render.\n");
    videoBuffer=NULL;
}
/**
    \fn simpleRender
*/
simpleRender::~simpleRender()
{
    ADM_info("Destroying simple render.\n");
    if(videoBuffer) delete [] videoBuffer;
    videoBuffer=NULL;
}

/**
    \fn stop
*/
bool simpleRender::stop(void)
{
    ADM_info("stopping simple render.\n");
    return true;
}
/**
    \fn refresh
*/
bool simpleRender::refresh(void)
{
     MUI_rgbDraw(MUI_getDrawWidget(),displayWidth,displayHeight,videoBuffer);
     return true;
}
/**
    \fn displayImage
*/
bool simpleRender::displayImage(ADMImage *pic)
{
    scaler->convertImage(pic,videoBuffer);
    // Display RGB data
    MUI_rgbDraw(MUI_getDrawWidget(),displayWidth,displayHeight,videoBuffer);
    return true;
}
#if !(ADM_UI_TYPE_BUILD == ADM_UI_QT4)
    #define IVERT ADM_COLOR_BGR32A
#else
    #define IVERT ADM_COLOR_RGB32A
#endif
/**
    \fn cleanup
*/
bool simpleRender::cleanup(void)
{
    if(videoBuffer) delete [] videoBuffer;
    videoBuffer=NULL;
    if(scaler) delete scaler;
    scaler=NULL;
    return true;
}
/**
    \fn allocateStuff
*/
bool simpleRender::allocateStuff(void)
{
        cleanup();
        scaler=new ADMColorScalerFull(ADM_CS_BICUBIC,imageWidth,imageHeight,displayWidth,displayHeight,
            ADM_COLOR_YV12,
            IVERT
        );
        videoBuffer=new uint8_t[displayWidth*displayHeight*4];
        return true;
}

/**
    \fn changeZoom
*/
bool simpleRender::changeZoom(renderZoom newZoom)
{
        ADM_info("changing zoom, simple render.\n");
        calcDisplayFromZoom(newZoom);
        currentZoom=newZoom;
        allocateStuff();
        return true;
}
/**
    \fn changeZoom
*/
bool simpleRender::init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom)
{
    info=*window;
    baseInit(w,h,zoom);
    ADM_info("init, simple render. w=%d, h=%d,zoom=%d\n",(int)w,(int)h,(int)zoom);
    allocateStuff();
    return true;
}
