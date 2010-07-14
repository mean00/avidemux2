/***************************************************************************
    \file GUI_vdpauRender.cpp
    \author mean fixounet@free.fr (C) 2010

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


#include "GUI_render.h"

#include "GUI_accelRender.h"
#include "GUI_vdpauRender.h"
#include "ADM_coreVdpau/include/ADM_coreVdpau.h"
static VdpOutputSurface surface[2]={VDP_INVALID_HANDLE,VDP_INVALID_HANDLE};
static int currentSurface=0;
static VdpPresentationQueue queue=VDP_INVALID_HANDLE;
//________________Wrapper around Xv_______________
/**
    \fn vdpauRender
*/
vdpauRender::vdpauRender( void )
{

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
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceCreate(VDP_RGBA_FORMAT_B8G8R8A8,w,h,&surface[0])) 
    {
        ADM_error("Cannot create outputSurface0\n");
        goto badInit;
    }
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceCreate(VDP_RGBA_FORMAT_B8G8R8A8,w,h,&surface[1])) 
    {
        ADM_error("Cannot create outputSurface1\n");
        goto badInit;
    }
    if(VDP_STATUS_OK!=admVdpau::presentationQueueCreate(&queue)) 
    {
        ADM_error("Cannot create queue\n");
        goto badInit;
    } 

    return true;
badInit:
    
    return false;
}
/**
    \fn cleanup
*/
bool vdpauRender::cleanup(void)
{
    if(surface[0]!=VDP_INVALID_HANDLE)  admVdpau::outputSurfaceDestroy(surface[0]);
    if(surface[1]!=VDP_INVALID_HANDLE)  admVdpau::outputSurfaceDestroy(surface[1]);
    if(queue!=VDP_INVALID_HANDLE)  admVdpau::presentationQueueDestroy(queue);
    surface[0]=surface[1]=VDP_INVALID_HANDLE;
    queue=VDP_INVALID_HANDLE;
}
/**
    \fn stop
*/
bool vdpauRender::stop(void)
{
	 
	 printf("[Vdpau]Xv end\n");
     cleanup();
	 return 1;
}
/**
    \fn displayImage
*/
bool vdpauRender::displayImage(ADMImage *pic)
{
    // Blit pic into our next image
    int next=currentSurface^1;
    uint32_t pitches[3];
    uint8_t *planes[3];
    pic->GetPitches(pitches);
    pic->GetReadPlanes(planes);
    if(VDP_STATUS_OK!=admVdpau::outPutSurfacePutBitsYV12( 
            surface[next],
            planes,pitches))
    {
        ADM_warning("Cannot putbits\n");
        return false;
    }
    // Display!
    if(VDP_STATUS_OK!=admVdpau::presentationQueueDisplay(queue,surface[next]))
    {
        ADM_warning("Cannot display on presenation queue\n");
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
        ADM_info("[Vdpau]changing zoom, xv render.\n");
        calcDisplayFromZoom(newZoom);
        currentZoom=newZoom;
        return true;
}
/**
    \fn refresh
*/
bool vdpauRender::refresh(void)
{
    // since we dont know how to redraw without help, ask above
    ADM_info("[Vdpau]refresh\n");
    if(VDP_STATUS_OK!=admVdpau::presentationQueueDisplay(queue,surface[currentSurface]))
    {
        ADM_warning("[refresh]Cannot display on presenation queue\n");
        return false;
    }
    renderCompleteRedrawRequest();
    return true;
}

#endif
