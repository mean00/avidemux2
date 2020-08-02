/***************************************************************************
    \file GUI_libvaRender.cpp
    \author mean fixounet@free.fr (C) 2013
    \brief  Use libva as renderer with hw rescaling.

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#ifdef USE_LIBVA
extern "C" {
#include "libavcodec/avcodec.h"
}


#include "GUI_render.h"

#include "GUI_accelRender.h"
#include "GUI_libvaRender.h"

static ADM_vaSurface *lastSurface=NULL;


VideoRenderBase *spawnLIBVARender()
{
    return new libvaRender();
}

//________________Wrapper around Xv_______________
/**
    \fn libvaRender
*/
libvaRender::libvaRender( void )
{
    mySurface[0]=mySurface[1]=NULL;;
    toggle=0;
    lastSurface=NULL;       
#if 0    
    currentFmt=VA_FOURCC_P010;
#else    
    currentFmt=VA_RT_FORMAT_YUV420;
#endif            
}
/**
    \đn dtor
*/
libvaRender::~libvaRender()
{
    cleanup();
}
/**
 *  \fn rescaleDisplay
 */
void libvaRender::rescaleDisplay(void)
{
    double dw=displayWidth;
    double dh=displayHeight;
    dw*=info.scalingFactor;
    dh*=info.scalingFactor;
    displayWidth=(uint32_t)(dw+0.5);
    displayHeight=(uint32_t)(dh+0.5);
}
/**
 * \fn realloc
 * @param newFormat
 * @return 
 */
bool libvaRender::realloc(int newFormat)
{
    cleanup();
    currentFmt=newFormat;
    for(int i=0;i<2;i++)
    {
        VASurfaceID surface=admLibVA::allocateSurface(imageWidth,imageHeight,currentFmt);
        if(surface==VA_INVALID)
        {
             ADM_warning("[libva] cannot allocate surface\n");
            return false;
        }

        mySurface[i]=new ADM_vaSurface(imageWidth,imageHeight);
        mySurface[i]->surface=surface;
    }    
    return true;
}


/**
    \fn init
*/
bool libvaRender::init( GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom)
{
    ADM_info("[libva]Xv start\n");
    info=*window;
    if(admLibVA::isOperationnal()==false)
    {
        ADM_warning("[libva] Not operationnal\n");
        return false;
    }
    for(int i=0;i<2;i++)
    {
        VASurfaceID surface=admLibVA::allocateSurface(w,h,currentFmt);
        if(surface==VA_INVALID)
        {
             ADM_warning("[libva] cannot allocate surface\n");
            return false;
        }

        mySurface[i]=new ADM_vaSurface(w,h);
        mySurface[i]->surface=surface;
    }    
    
    baseInit(w,h,zoom);
    rescaleDisplay();
    return true;
}
/**
    \fn cleanup
*/
bool libvaRender::cleanup(void)
{
    lastSurface=NULL;
    for(int i=0;i<2;i++)
    {
        if(mySurface[i])
        {
            delete mySurface[i];
            mySurface[i]=NULL;
        }
    }
    return true;
}
/**
    \fn stop
*/
bool libvaRender::stop(void)
{
	 
     ADM_info("[libva]Vdpau render end\n");
     cleanup();
     return true;
}
/**
    \fn displayImage
*/
bool libvaRender::displayImage(ADMImage *pic)
{
    // if input is already a VA surface, no need to reupload it...
    if(pic->refType==ADM_HW_LIBVA)
    {
        ADM_vaSurface *img=(ADM_vaSurface *)pic->refDescriptor.refHwImage;
        admLibVA::putX11Surface(img,info.systemWindowId,imageWidth,imageHeight,displayWidth,displayHeight);
        lastSurface=img;
    }else
    {
        if(!mySurface[0] || !mySurface[1])
        {
            ADM_warning("[VARender] No surface\n");
            return false;
        }
        ADM_vaSurface *dest=mySurface[toggle];
        toggle^=1;
        if(false==dest->fromAdmImage(pic))
        {
            ADM_warning("VaRender] Failed to upload pic \n");
            return false;
        }
        admLibVA::putX11Surface(dest,info.systemWindowId,imageWidth,imageHeight,displayWidth,displayHeight);
        lastSurface=dest;
    }
    return true;
}

/**
    \fn changeZoom
*/
bool libvaRender::changeZoom(float newZoom)
{
    ADM_info("[libvaRender] Changing zoom.\n");
    calcDisplayFromZoom(newZoom);
    currentZoom=newZoom;
    rescaleDisplay();
    return true;
}
/**
    \fn refresh
*/
bool libvaRender::refresh(void)
{
    // since we dont know how to redraw without help, ask above
    ADM_info("[libva]Rrefresh\n");
    // This is a little bit dangerous, lastSurface might be gone
    if(lastSurface)
        admLibVA::putX11Surface(lastSurface,info.systemWindowId,imageWidth,imageHeight,displayWidth,displayHeight);
    else
        renderCompleteRedrawRequest();
    return true;
}
#endif
