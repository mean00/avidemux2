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


static int  sdlDriverIndex=-1;
/**
 * \class sdlRenderImpl
 * \brief implementation
 */
class sdlRenderImpl: public VideoRenderBase
{
  protected:
              bool     useYV12;
  public:
                             sdlRenderImpl( void ) ;
              virtual        ~sdlRenderImpl();
              virtual	bool init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom);
              virtual	bool stop(void);				
              virtual   bool displayImage(ADMImage *pic);
              virtual   bool changeZoom(renderZoom newZoom);
              virtual   bool usingUIRedraw(void) {return true;};
              virtual   bool refresh(void) ;
protected:
                        bool cleanup(void);
                        bool sdl_running;
                        SDL_Window   *sdl_window;
                        SDL_Renderer *sdl_renderer;
                        SDL_Texture  *sdl_texture;
                        #ifdef _WIN32
                        HWND sdlWin32;
                        #endif
                        
};


static sdlRenderImpl *impl=NULL;

/**
 */

static std::vector<sdlDriverInfo>listOfSDLDrivers;
/**
 * 
 * @return 
 */
static bool initDrivers()
{
    listOfSDLDrivers.clear();
    int nbDriver=SDL_GetNumRenderDrivers();
   
    for(int i=0;i<nbDriver;i++)
    {
        SDL_RendererInfo info;
        SDL_GetRenderDriverInfo(i,&info);
        sdlDriverInfo sdlInfo;
        sdlInfo.index=i;
        sdlInfo.driverName=std::string(info.name);
        sdlInfo.flags=info.flags;
        listOfSDLDrivers.push_back(sdlInfo);
        printf("[SDK]Found driver [%d] %s, with flags=%x\n",i,info.name,info.flags);
        if(info.flags & SDL_RENDERER_SOFTWARE) // by default we peek the software one
            sdlDriverIndex=i;
    }
    return true;
}

/**
 * 
 */
sdlRender::sdlRender()
{
    if(!impl)
        impl=new sdlRenderImpl;
}
/**
 * 
 * @param window
 * @param w
 * @param h
 * @param zoom
 * @return 
 */
bool sdlRender::init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom)
{
    ADM_assert(impl);
    return impl->init(window,w,h,zoom);
}
/**
 * 
 * @return 
 */
bool sdlRender::stop()
{
    ADM_assert(impl);
    return impl->stop();
}
/**
 * 
 * @param pic
 * @return 
 */
bool  sdlRender::displayImage(ADMImage *pic)
{
    ADM_assert(impl);
    return impl->displayImage(pic);
}
/**
 * 
 * @param newZoom
 * @return 
 */
bool  sdlRender::changeZoom(renderZoom newZoom)
{
    ADM_assert(impl);
    return impl->changeZoom(newZoom);
}
/**
 * 
 * @return 
 */
bool  sdlRender::usingUIRedraw()
{
    ADM_assert(impl);
    return impl->usingUIRedraw();
}
/**
 * 
 * @return 
 */
bool sdlRender::refresh(void) 
{
    ADM_assert(impl);
    return impl->refresh();
}
/**
 * 
 * @return 
 */
const std::vector<sdlDriverInfo> &getListOfSdlDrivers()
{
    return listOfSDLDrivers;
}
/**
 * 
 * @return 
 */
std::string  getSdlDriverName()
{
    return listOfSDLDrivers[sdlDriverIndex].driverName;
}
/**
 * 
 * @param name
 * @return 
 */
bool  setSdlDriverByName(const std::string &name)
{
    int defaultOne=-1;
    ADM_info("[SDL] Trying to switch to SDL driver %s\n",name.c_str());
    for(int i=0;i<listOfSDLDrivers.size();i++)
    {
        if(!listOfSDLDrivers[i].driverName.compare(name))
        {
            ADM_info("[SDL] Picked up driver <%s>\n",listOfSDLDrivers[i].driverName.c_str());
            sdlDriverIndex=i;
            return true;
        }
        if(listOfSDLDrivers[i].flags & SDL_RENDERER_SOFTWARE) // by default we take the software one
            defaultOne=i;
    }
    ADM_warning("No suitable driver found\n");
    ADM_assert(defaultOne>=0);
    sdlDriverIndex=defaultOne;
    return false;
}


//******************************************
/**
    \fn sdlRender
*/
sdlRenderImpl::sdlRenderImpl( void)
{
        useYV12=true;
        sdl_running=false;
        ADM_info("[SDL] Init rendered\n");
        sdl_window=NULL;
        sdl_renderer=NULL;
        sdl_texture=NULL;
}
/**
 * 
 */
sdlRenderImpl::~sdlRenderImpl()
{
    stop();
}

/**
    \fn stop
*/
bool sdlRenderImpl::stop( void)
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
bool sdlRenderImpl::init( GUI_WindowInfo * window, uint32_t w, uint32_t h,renderZoom zoom)
{
    ADM_info("[SDL] Initialising video subsystem\n");

    int bpp;
    int flags;
    baseInit(w,h,zoom);

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
		ADM_warning("[SDL] FAILED initialising video subsystem\n");
		ADM_warning("[SDL] ERROR: %s\n", SDL_GetError());

        return 0;
    }
    ADM_info("[SDL] Video subsystem init ok\n");

    sdl_running=1;
    ADM_info("[SDL] Creating window (at %x,%d)\n",window->x,window->y);
    
    int nbDriver=listOfSDLDrivers.size();
    if(!nbDriver)
    {
        ADM_warning("[SDL] No driver loaded\n");
        return false;
    }
    if(sdlDriverIndex==-1)
    {
        ADM_warning("No suitable driver found\n");
        return false;
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
bool sdlRenderImpl::cleanup()
{
    if(sdl_texture)
    {
        SDL_DestroyTexture(sdl_texture);
        sdl_texture=NULL;
    }
    if(sdl_renderer)
    {
        SDL_DestroyRenderer(sdl_renderer);
        sdl_renderer=NULL;
    }
    if(sdl_window)
    {
        //SDL_DestroyWindow(sdl_window);
        sdl_window=NULL;
    }
    return true;
}
/**
    \fn displayImage
*/
bool sdlRenderImpl::displayImage(ADMImage *pic)
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
    SDL_RenderClear(sdl_renderer);
    SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
    refresh();
    return true;
}
/**
 * 
 * @return 
 */
bool sdlRenderImpl::refresh(void)
{
    if(!sdl_texture)
        return false;
    SDL_RenderPresent(sdl_renderer);
    return true;
}
/**
    \fn changeZoom
*/
bool sdlRenderImpl::changeZoom(renderZoom newZoom)
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
bool initSdl(const std::string &sdlDriverName)
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
    initDrivers();
    setSdlDriverByName(sdlDriverName);
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

    if(impl)
    {
        delete impl;
        impl=NULL;
    }

}
#endif
