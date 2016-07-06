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
    myImage=QImage(videoBuffer,displayWidth,displayHeight,QImage::Format_RGB32).copy(0,0,displayWidth,displayHeight);
    lock.unlock();
    refresh();
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
    videoWidget=(ADM_Qvideo *)info.widget;
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
        int x=rec.x();
        int y=rec.y();
        int w=rec.width();
        int h=rec.height();
        painter.drawImage(x,y,myImage,x,y,w,h);
    }else
    {
        ADM_warning("Painter inactive!\n");
    }
    return true;
}