/***************************************************************************
    copyright            : (C) 2006/2015 by mean
    email                : fixounet@free.fr
 * 
 * SDL2 version : See http://wiki.libsdl.org/MigrationGuide
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************///
#include "config.h"

#if defined(USE_SDL)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
// get rid of warnings due to different definitions
#undef HAVE_INTTYPES_H
#undef HAVE_MALLOC_H
#undef HAVE_STDINT_H
#undef HAVE_SYS_TYPES_H
#ifdef __HAIKU__
#include "SDL2/SDL.h"
#else
#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#endif
}
#include "ADM_default.h"
#include "ADM_colorspace.h"
#include "GUI_render.h"
#include "GUI_accelRender.h"
#include "GUI_sdlRender.h"


//******************************************
static uint8_t sdl_running=0;
//static SDL_Overlay *sdl_overlay=NULL;
static SDL_Surface *sdl_display=NULL;
static SDL_Window  *sdl_window=NULL;
static SDL_Renderer *sdl_renderer=NULL;
static SDL_Texture *sdl_texture=NULL;
static SDL_Rect disp;
#ifdef _WIN32
HWND sdlWin32;
#endif
/**
    \fn sdlRender
*/
sdlRender::sdlRender( void)
{
        useYV12=true;
        ADM_info("[SDL] Init rendered\n");
}
/**
    \fn stop
*/
bool sdlRender::stop( void)
{
        cleanup();
        if(sdl_running)
        {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }
        sdl_running=0;
        ADM_info("[SDL] Video subsystem closed and destroyed\n");
        return true;
}
/**
    \fn init
*/
bool sdlRender::init( GUI_WindowInfo * window, uint32_t w, uint32_t h,renderZoom zoom)
{
    ADM_info("[SDL] Initialising video subsystem\n");

    int bpp;
    int flags;
    baseInit(w,h,zoom);
    // Ask for the position of the drawing window at start
    disp.w=w;
    disp.h=h;
    disp.x=window->x;
    disp.y=window->y;

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
		ADM_warning("[SDL] FAILED initialising video subsystem\n");
		ADM_warning("[SDL] ERROR: %s\n", SDL_GetError());

        return 0;
    }
    ADM_info("[SDL] Video subsystem init ok\n");

    sdl_running=1;
    ADM_info("[SDL] Creating window (at %x,%d)\n",window->x,window->y);
    
    int nbDriver=SDL_GetNumRenderDrivers();
    for(int i=0;i<nbDriver;i++)
    {
        SDL_RendererInfo info;
        SDL_GetRenderDriverInfo(i,&info);
        ADM_info("[%d] %s\n",i,info.name);
    }
    
#if 0
    sdl_window = SDL_CreateWindow("avidemux_sdl2",
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          w, h,
                          SDL_WINDOW_BORDERLESS | SDL_WINDOW_FOREIGN*1);    
    SDL_SetWindowPosition(sdl_window,window->x,window->y);
#else
    sdl_window=SDL_CreateWindowFrom((void*)window->window);    
#endif    
    
    if(!sdl_window)
    {
        ADM_info("[SDL] Creating window failed!\n");
        cleanup();
        return false;
    }
    int sdlDriverIndex=2;
    sdl_renderer = SDL_CreateRenderer(sdl_window, sdlDriverIndex, SDL_RENDERER_ACCELERATED |  SDL_RENDERER_PRESENTVSYNC);
    if(!sdl_renderer)
        sdl_renderer = SDL_CreateRenderer(sdl_window, sdlDriverIndex, 0);
    if(!sdl_renderer)
    {
        ADM_warning("[SDL] FAILED to create a renderer\n");
        cleanup();
        return false;
    }
    
    sdl_texture = SDL_CreateTexture(sdl_renderer,
                               SDL_PIXELFORMAT_YV12,
                               SDL_TEXTUREACCESS_STREAMING,
                               w, h);
    if(sdl_texture)
    {
        useYV12=true;
    }else
    {
        useYV12=false;
        sdl_texture = SDL_CreateTexture(sdl_renderer,
                               SDL_PIXELFORMAT_ARGB8888,
                               SDL_TEXTUREACCESS_STREAMING,
                               w, h);
        if(!sdl_texture)
        {
            ADM_warning("[SDL] FAILED to create a texture (rgb)\n");
            cleanup();
            return false;
        }
    }
    changeZoom(zoom);
    return true;
}
/**
 * 
 * @return 
 */
bool sdlRender::cleanup()
{
    return true;
}
/**
    \fn displayImage
*/
bool sdlRender::displayImage(ADMImage *pic)
{
    if(!sdl_texture)
        return false;
    if(useYV12)
    {
        uint32_t imagePitch[3];
        uint8_t  *imagePtr[3];
        pic->GetPitches(imagePitch);
        pic->GetWritePlanes(imagePtr);
        SDL_UpdateYUVTexture(sdl_texture,
                                             NULL,
                                             imagePtr[0], imagePitch[0],
                                             imagePtr[2], imagePitch[2],
                                             imagePtr[1], imagePitch[1]);

    }else
    { // YUYV
        ADM_warning("[SDL] YUYV disabled\n");
        return false;
    }

    refresh();
    return true;
}
/**
 * 
 * @return 
 */
bool sdlRender::refresh(void)
{
    if(!sdl_texture)
        return false;
    SDL_RenderClear(sdl_renderer);
    SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
    SDL_RenderPresent(sdl_renderer);
    return true;
}
/**
    \fn changeZoom
*/
bool sdlRender::changeZoom(renderZoom newZoom)
{
        ADM_info("changing zoom, sdl render.\n");
        calcDisplayFromZoom(newZoom);
        currentZoom=newZoom;
        if(sdl_renderer)
        {
            float scaleX=(float)displayWidth/(float)imageWidth;
            float scaleY=(float)displayWidth/(float)imageHeight;
            
            SDL_RenderSetScale(sdl_renderer,   scaleX, scaleY);
            SDL_SetWindowSize(sdl_window,displayWidth,displayHeight);
        }
        return true;
}
/**
    \fn initSdl
*/
bool initSdl(int videoDevice)
{
    printf("\n");
    quitSdl();
    SDL_version version;
    SDL_version *ver=&version;
    
    SDL_GetVersion(ver);
    
    int sdl_version = (ver->major*1000)+(ver->minor*100) + (ver->patch);

    ADM_info("[SDL] Version: %u.%u.%u\n",ver->major, ver->minor, ver->patch);

    uint32_t sdlInitFlags;
    sdlInitFlags = SDL_INIT_AUDIO |SDL_INIT_VIDEO ;
    ADM_info("[SDL] Initialisation ");

    if (SDL_Init(sdlInitFlags))
    {
            ADM_info("\tFAILED\n");
            ADM_info("[SDL] ERROR: %s\n", SDL_GetError());
            return false;
    }
    ADM_info("\tsucceeded\n");

    const char *driverName=SDL_GetVideoDriver(0);
    if(driverName)
    {
            ADM_info("[SDL] Video Driver: %s\n", driverName);
    }
    return true;
	
}
/**
    \fn quitSdl
*/
void quitSdl(void)
{
	if (SDL_WasInit(SDL_INIT_EVERYTHING))
	{
		ADM_info("[SDL] Quitting...\n");
		SDL_Quit();
	}
}
#endif
