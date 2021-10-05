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
#include "QPainter"
#include "QPaintEngine"

extern void *MUI_getDrawWidget(void);


VideoRenderBase *spawnSimpleRender()
{
    return new simpleRender();
}


/**
    \fn simpleRender
*/
simpleRender::simpleRender()
{
    ADM_info("creating simple render.\n");
    videoBuffer=NULL;
    paintEngineType=-1;
    videoWidget=NULL;
}
/**
    \fn simpleRender
*/
simpleRender::~simpleRender()
{
    admScopedMutex autoLock(&lock);
    videoWidget->setDrawer(NULL);
    videoWidget->useExternalRedraw(true);
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
     videoWidget->repaint();
     return true;
}
/**
    \fn displayImage
*/
bool simpleRender::displayImage(ADMImage *pic)
{
    scaler->convertImage(pic,videoBuffer);
    lock.lock();
    int stride=ADM_IMAGE_ALIGN(displayWidth*4);
    myImage=QImage(videoBuffer,displayWidth,displayHeight,stride,QImage::Format_RGB32).copy(0,0,displayWidth,displayHeight);
#if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
    myImage.setDevicePixelRatio(info.scalingFactor);
#endif
    lock.unlock();
    refresh();
    return true;
}
//#if (ADM_UI_TYPE_BUILD == ADM_UI_QT4)
    #define IVERT ADM_PIXFRMT_BGR32A
//#else
//    #define IVERT ADM_COLOR_RGB32A
//#endif
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
            ADM_PIXFRMT_YV12,
            IVERT
        );
        uint32_t sz=ADM_IMAGE_ALIGN(displayWidth*4);
        sz*=displayHeight;
        videoBuffer=new uint8_t[sz];
        return true;
}

/**
    \fn changeZoom
*/
bool simpleRender::changeZoom(float newZoom)
{
        ADM_info("changing zoom, simple render.\n");
        calcDisplayFromZoom(newZoom);
#if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
        displayWidth*=info.scalingFactor;
        displayHeight*=info.scalingFactor;
#endif
        currentZoom=newZoom;
        allocateStuff();
        return true;
}
/**
    \fn changeZoom
*/
bool simpleRender::init( GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom)
{
    info=*window;
    baseInit(w,h,zoom);
#if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
    displayWidth*=info.scalingFactor;
    displayHeight*=info.scalingFactor;
#endif
    ADM_info("init, simple render. w=%d, h=%d,zoom=%.4f\n",(int)w,(int)h,zoom);
    allocateStuff();
    videoWidget=(ADM_Qvideo *)info.widget;
    videoWidget->useExternalRedraw(false);
    videoWidget->setDrawer(this);
    return true;
}

/**
 * \brief This is the callback called when the display wants to redraw
 * @param widget
 * @param ev
 * @return 
 */
bool simpleRender::draw(QWidget *widget, QPaintEvent *ev)
{
    admScopedMutex autoLock(&lock); 
    QPainter painter(widget);
    if (painter.isActive())
    {
        const QRect rec=ev->rect();
        painter.drawImage(rec,myImage);
    }else
    {
        ADM_warning("Painter inactive!\n");
    }
    return true;
}
