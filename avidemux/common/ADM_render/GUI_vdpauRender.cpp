/***************************************************************************
    \file GUI_vdpauRender.cpp
    \author mean fixounet@free.fr (C) 2010
    \brief  Use vdpau as renderer with hw rescaling.

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
#ifdef USE_VDPAU
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/vdpau.h"
}


#include "GUI_render.h"

#include "GUI_accelRender.h"
#include "GUI_vdpauRender.h"
#include "ADM_coreVideoCodec/ADM_hwAccel/ADM_coreVdpau/include/ADM_coreVdpau.h"
#include "ADM_videoCodec/include/ADM_ffmpeg_vdpau_internal.h"
static VdpOutputSurface     surface[2]={VDP_INVALID_HANDLE,VDP_INVALID_HANDLE};
static VdpVideoSurface      input=VDP_INVALID_HANDLE;
static VdpVideoMixer        mixer=VDP_INVALID_HANDLE;
static int                  currentSurface=0;
static VdpPresentationQueue queue=VDP_INVALID_HANDLE;
//________________Wrapper around Xv_______________
/**
    \fn vdpauRender
*/
vdpauRender::vdpauRender( void )
{
    
}
/**
    \đn dtor
*/
vdpauRender::~vdpauRender()
{

    cleanup();
}

/**
    \fn init
*/
bool vdpauRender::init( GUI_WindowInfo * window, uint32_t w, uint32_t h,renderZoom zoom)
{
	ADM_info("[Vdpau]Xv start\n");
    info=*window;
    if(admVdpau::isOperationnal()==false)
    {
        ADM_warning("[Vdpau] Not operationnal\n");
    }
    baseInit(w,h,zoom);
    // Create couple of outputSurface
    surface[0]=surface[1]=VDP_INVALID_HANDLE;
    currentSurface=0;
    if(!reallocOutputSurface(displayWidth,displayHeight))
    {
        goto badInit;
    }
    if(VDP_STATUS_OK!=admVdpau::surfaceCreate(w,h,&input)) 
    {
        ADM_error("Cannot create input Surface\n");
        goto badInit;
    }
    if(VDP_STATUS_OK!=admVdpau::presentationQueueCreate(&queue)) 
    {
        ADM_error("Cannot create queue\n");
        goto badInit;
    } 
    if(VDP_STATUS_OK!=admVdpau::mixerCreate(w,h,&mixer)) 
    {
        ADM_error("Cannot create mixer\n");
        goto badInit;
    } 

    return true;
badInit:
    
    return false;
}
/**
    \fn reallocOutputSurface
*/
bool vdpauRender::reallocOutputSurface(uint32_t tgtWidth, uint32_t tgtHeight)
{
    if(surface[0]!=VDP_INVALID_HANDLE)  admVdpau::outputSurfaceDestroy(surface[0]);
    if(surface[1]!=VDP_INVALID_HANDLE)  admVdpau::outputSurfaceDestroy(surface[1]);
    surface[0]=surface[1]=VDP_INVALID_HANDLE;
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceCreate(VDP_RGBA_FORMAT_B8G8R8A8,tgtWidth,tgtHeight,&surface[0])) 
    {
        ADM_error("Cannot create outputSurface0\n");
        return false;
    }
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceCreate(VDP_RGBA_FORMAT_B8G8R8A8,tgtWidth,tgtHeight,&surface[1])) 
    {
        ADM_error("Cannot create outputSurface1\n");
        return false;
    }
    return true;
}
/**
    \fn cleanup
*/
bool vdpauRender::cleanup(void)
{
    if(input!=VDP_INVALID_HANDLE) admVdpau::surfaceDestroy(input);
    if(surface[0]!=VDP_INVALID_HANDLE)  admVdpau::outputSurfaceDestroy(surface[0]);
    if(surface[1]!=VDP_INVALID_HANDLE)  admVdpau::outputSurfaceDestroy(surface[1]);
    if(queue!=VDP_INVALID_HANDLE)  admVdpau::presentationQueueDestroy(queue);
    if(mixer!=VDP_INVALID_HANDLE) admVdpau::mixerDestroy(mixer);
    surface[0]=surface[1]=VDP_INVALID_HANDLE;
    queue=VDP_INVALID_HANDLE;
    input=VDP_INVALID_HANDLE;
    mixer=VDP_INVALID_HANDLE;
    return true;
}
/**
    \fn stop
*/
bool vdpauRender::stop(void)
{
	 
	 printf("[Vdpau]Vdpau render end\n");
     cleanup();
	 return 1;
}
/**
    \fn displayImage
*/
bool vdpauRender::displayImage(ADMImage *pic)
{
    // Blit pic into our video Surface
    VdpVideoSurface myInput=input;
    int next=currentSurface^1;
    uint32_t pitches[3];
    uint8_t *planes[3];
    pic->GetPitches(pitches);
    pic->GetReadPlanes(planes);

    // Put out stuff in input...
    // if input is already a VDPAU surface, no need to reupload it...
    if(pic->refType==ADM_HW_VDPAU)
    {
        // cookie is a render...
        struct vdpau_render_state *rndr = (struct vdpau_render_state *)pic->refDescriptor.refCookie;
        myInput=rndr->surface;
        //printf("Skipping blit surface=%d\n",(int)myInput);
    }else
    {
        //printf("Blitting surface\n");
        if(VDP_STATUS_OK!=admVdpau::surfacePutBits( 
                input,
                planes,pitches))
        {
            ADM_warning("[Vdpau] video surface : Cannot putbits\n");
            return false;
        }
    }
    // Call mixer...
    if(VDP_STATUS_OK!=admVdpau::mixerRender( mixer,myInput,surface[next], pic->_width,pic->_height))

    {
        ADM_warning("[Vdpau] Cannot mixerRender\n");
        return false;
    }
    // Display!
    if(VDP_STATUS_OK!=admVdpau::presentationQueueDisplay(queue,surface[next]))
    {
        ADM_warning("[Vdpau] Cannot display on presenation queue\n");
        return false;
    }
    currentSurface=next;
    return true;
}

/**
    \fn changeZoom
*/
bool vdpauRender::changeZoom(renderZoom newZoom)
{
        ADM_info("[Vdpau]changing zoom.\n");
        calcDisplayFromZoom(newZoom);
        currentZoom=newZoom;
        if(!reallocOutputSurface(displayWidth,displayHeight))
        {
            ADM_error("[VdpauRender] Change zoome failed\n");
        }
        return true;
}
/**
    \fn refresh
*/
bool vdpauRender::refresh(void)
{
    // since we dont know how to redraw without help, ask above
    ADM_info("[Vdpau]Rrefresh\n");
    if(VDP_STATUS_OK!=admVdpau::presentationQueueDisplay(queue,surface[currentSurface]))
    {
        ADM_warning("[Vdpau] Refresh : Cannot display on presenation queue\n");
        return false;
    }
    renderCompleteRedrawRequest();
    return true;
}
#endif
