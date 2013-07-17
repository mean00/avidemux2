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


//________________Wrapper around Xv_______________
/**
    \fn libvaRender
*/
libvaRender::libvaRender( void )
{
    mySurface[0]=mySurface[1]=NULL;;
    myImage=NULL;
    toggle=0;
    
    
}
/**
    \đn dtor
*/
libvaRender::~libvaRender()
{

    cleanup();
}

/**
    \fn init
*/
bool libvaRender::init( GUI_WindowInfo * window, uint32_t w, uint32_t h,renderZoom zoom)
{
    ADM_info("[libva]Xv start\n");
    info=*window;
    if(admLibVA::isOperationnal()==false)
    {
        ADM_warning("[libva] Not operationnal\n");
        return false;
    }
    directUploadSupported=true;
    for(int i=0;i<2;i++)
    {
        VASurfaceID surface=admLibVA::allocateSurface(w,h);
        if(surface==VA_INVALID)
        {
             ADM_warning("[libva] cannot allocate surface\n");
            return false;
        }

        mySurface[i]=new ADM_vaImage(NULL,w,h);
        mySurface[i]->surface=surface;
    }    
    myImage=admLibVA::allocateNV12Image(w,h);
    if(!myImage)
    {
         ADM_warning("[libva] cannot allocate image\n");
        return false;
    }

    
    baseInit(w,h,zoom);
    return true;
}
/**
    \fn cleanup
*/
bool libvaRender::cleanup(void)
{
    for(int i=0;i<2;i++)
    {
        if(mySurface[i])
        {
            delete mySurface[i];
            mySurface[i]=NULL;
        }
    }
    if(myImage)
    {
        admLibVA::destroyImage(myImage);
        myImage=NULL;
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
        ADM_vaImage *img=(ADM_vaImage *)pic->refDescriptor.refInstance;
        admLibVA::putX11Surface(img,info.window,displayWidth,displayHeight);
    }else
    {
        if(!mySurface[0] || !mySurface[1])
        {
            ADM_warning("[VARender] No surface\n");
            return false;
        }
        if(!myImage)
        {
            ADM_warning("[VARender] No image\n");
            return false;
        }
        ADM_vaImage *dest=mySurface[toggle];
        toggle^=1;
again:
        if(directUploadSupported)
        {
            if(false==admLibVA::uploadToSurface(pic,dest))
            {
                ADM_warning("VaRender] Failed to upload pic directly\n");
                directUploadSupported=false;
                goto again;
            }
        }else
        {
             if(false==admLibVA::uploadToImage(pic,myImage))
             {
                ADM_warning("[VARender] uploading to image failed\n");
                return false;
             }
            // then VAImage to VASurface
            if(false==admLibVA::imageToSurface(myImage,dest))
            {
                ADM_warning("[VARender] uploading to surface failed\n");
                return false;
            }
        }
        admLibVA::putX11Surface(dest,info.window,displayWidth,displayHeight);
    }
    return true;
}

/**
    \fn changeZoom
*/
bool libvaRender::changeZoom(renderZoom newZoom)
{
        ADM_info("[libva]changing zoom.\n");
        calcDisplayFromZoom(newZoom);
        currentZoom=newZoom;
        return true;
}
/**
    \fn refresh
*/
bool libvaRender::refresh(void)
{
    // since we dont know how to redraw without help, ask above
    ADM_info("[libva]Rrefresh\n");
   
    renderCompleteRedrawRequest();
    return true;
}
#endif
