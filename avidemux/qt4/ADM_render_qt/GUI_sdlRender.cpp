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

extern "C"
{
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
#include "ADM_colorspace.h"
#include "ADM_default.h"
#include "GUI_accelRender.h"
#include "GUI_render.h"
#include "GUI_sdlRender.h"

static int sdlDriverIndex = -1;
static int sdlSoftwareDriverIndex = 0;
static std::vector<sdlDriverInfo> listOfSDLDrivers;
/**
 * \class sdlRenderImpl
 * \brief implementation
 */
class sdlRenderImpl : public VideoRenderBase
{
  protected:
    GUI_WindowInfo winfo;
    bool useYV12;
    int accel;

  public:
    sdlRenderImpl(void);
    virtual ~sdlRenderImpl();
    virtual bool init(GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom);
    virtual bool stop(void);
    virtual bool displayImage(ADMImage *pic);
    virtual bool changeZoom(float newZoom);
    virtual bool usingUIRedraw(void)
    {
        return false;
    };
    virtual bool refresh(void);
    const char *getName();

  protected:
    bool cleanup(void);
    void rescaleDisplay(void);
    bool sdl_running;
    SDL_Window *sdl_window;
    SDL_Renderer *sdl_renderer;
    SDL_Texture *sdl_texture;
};
/**
 */
VideoRenderBase *spawnSdlRender()
{
    return new sdlRender();
}
/**
 *
 * @param userdata
 * @param category
 * @param priority
 * @param message
 */
static void SDL_Logger(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    ADM_info("[SDL] %s\n", message);
}
/**
 *
 * @return
 */
static bool initDrivers()
{
    listOfSDLDrivers.clear();
    int nbDriver = SDL_GetNumRenderDrivers();

    for (int i = 0; i < nbDriver; i++)
    {
        SDL_RendererInfo info;
        SDL_GetRenderDriverInfo(i, &info);
        sdlDriverInfo sdlInfo;
        sdlInfo.index = i;
        if (info.name)
            sdlInfo.driverName = info.name;
        else
            sdlInfo.driverName = "Invalid driver";
        sdlInfo.flags = info.flags;
        listOfSDLDrivers.push_back(sdlInfo);
        ADM_info("[SDL] Found driver [%d] %s with flags = 0x%x\n", i, sdlInfo.driverName.c_str(), info.flags);
        if (info.flags & SDL_RENDERER_SOFTWARE) // by default we peek the software one
            sdlSoftwareDriverIndex = i;
    }
    sdlDriverIndex = sdlSoftwareDriverIndex;
    SDL_LogSetOutputFunction(SDL_Logger, NULL);
    return true;
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
 * @param name
 * @return
 */
bool setSdlDriverByName(const std::string &name)
{
    ADM_info("[SDL] Trying to switch to SDL driver %s\n", name.c_str());
    int nb = listOfSDLDrivers.size();
    for (int i = 0; i < nb; i++)
    {
        if (!listOfSDLDrivers[i].driverName.compare(name))
        {
            ADM_info("[SDL] Got it, selecting driver %d <%s>\n", i, listOfSDLDrivers[i].driverName.c_str());
            sdlDriverIndex = i;
            return true;
        }
    }
    ADM_warning("[SDL]No suitable driver found\n");
    sdlDriverIndex = sdlSoftwareDriverIndex;
    return false;
}

/**
 *
 * @return
 */
std::string getSdlDriverName()
{
    int nb = listOfSDLDrivers.size();
    ADM_assert(sdlDriverIndex < nb);
    return listOfSDLDrivers[sdlDriverIndex].driverName;
}
/**
 *
 */
sdlRender::sdlRender()
{
    impl = (void *)new sdlRenderImpl;
}
sdlRender::~sdlRender()
{
    if (impl)
    {
        sdlRenderImpl *i = (sdlRenderImpl *)impl;
        impl = NULL;
        delete i;
    }
}
const char *sdlRender::getName()
{
    if (impl)
    {
        return ((sdlRenderImpl *)impl)->getName();
    }
    return "SDL2";
}
/**
 *
 * @param window
 * @param w
 * @param h
 * @param zoom
 * @return
 */
bool sdlRender::init(GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom)
{
    sdlRenderImpl *im = (sdlRenderImpl *)impl;
    ADM_assert(im);
    return im->init(window, w, h, zoom);
}
/**
 *
 * @return
 */
bool sdlRender::stop()
{
    sdlRenderImpl *im = (sdlRenderImpl *)impl;
    ADM_assert(im);
    return im->stop();
}
/**
 *
 * @param pic
 * @return
 */
bool sdlRender::displayImage(ADMImage *pic)
{
    sdlRenderImpl *im = (sdlRenderImpl *)impl;
    ADM_assert(im);
    return im->displayImage(pic);
}
/**
 *
 * @param newZoom
 * @return
 */
bool sdlRender::changeZoom(float newZoom)
{
    sdlRenderImpl *im = (sdlRenderImpl *)impl;
    ADM_assert(im);
    return im->changeZoom(newZoom);
}
/**
 *
 * @return
 */
bool sdlRender::usingUIRedraw()
{
    sdlRenderImpl *im = (sdlRenderImpl *)impl;
    ADM_assert(im);
    return im->usingUIRedraw();
}
/**
 *
 * @return
 */
bool sdlRender::refresh(void)
{
    sdlRenderImpl *im = (sdlRenderImpl *)impl;
    ADM_assert(im);
    return im->refresh();
}

//******************************************
/**
    \fn sdlRender
*/
sdlRenderImpl::sdlRenderImpl(void)
{
    useYV12 = true;
    sdl_running = false;
    ADM_info("Creating.\n");
    sdl_window = NULL;
    sdl_renderer = NULL;
    sdl_texture = NULL;
    accel = -1;
    memset(&winfo, 0, sizeof(GUI_WindowInfo));
    winfo.scalingFactor = 1.;
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
bool sdlRenderImpl::stop(void)
{
    ADM_info("[SDL] Stopping\n");
    cleanup();
    if (sdl_running)
    {
        ADM_info("[SDL] Video subsystem closed\n");
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        sdl_running = 0;
    }

    return true;
}
/**
    \fn rescaleDisplay
*/
void sdlRenderImpl::rescaleDisplay(void)
{
    double dw, dh;

    dw = displayWidth;
    dh = displayHeight;
    dw *= winfo.scalingFactor;
    dh *= winfo.scalingFactor;
    displayWidth = (uint32_t)(dw + 0.5);
    displayHeight = (uint32_t)(dh + 0.5);
}
/**
    \fn init
*/
bool sdlRenderImpl::init(GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom)
{
    ADM_info("[SDL] Initializing video subsystem\n");

    int bpp;
    int flags;
    winfo = *window;
    baseInit(w, h, zoom);
    rescaleDisplay();

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
        ADM_warning("[SDL] Video subsystem init failed, error: %s\n", SDL_GetError());
        return false;
    }
    ADM_info("[SDL] Video subsystem init ok\n");

    sdl_running = true;
    ADM_info("[SDL] Creating window at (%d, %d)\n", window->x, window->y);

    int nbDriver = listOfSDLDrivers.size();
    if (!nbDriver)
    {
        ADM_warning("[SDL] No driver loaded\n");
        return false;
    }
    if (sdlDriverIndex == -1 || sdlDriverIndex >= nbDriver)
    {
        ADM_warning("[SDL] No available driver found\n");
        return false;
    }

    SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_OPENGL, "1");

#if 0
    sdl_window = SDL_CreateWindow("avidemux_sdl2",
                          winfo.x,
                          winfo.y,
                          displayWidth, displayHeight,
                          SDL_WINDOW_BORDERLESS | SDL_WINDOW_FOREIGN);
    //SDL_SetWindowPosition(sdl_window,window->x,window->y);
#else
    sdl_window = SDL_CreateWindowFrom((void *)window->systemWindowId);
#endif

    if (!sdl_window)
    {
        ADM_warning("[SDL] Creating window failed, error: %s\n", SDL_GetError());
        cleanup();
        return false;
    }

    ADM_info("Trying to create SDL renderer with driver %s\n", listOfSDLDrivers[sdlDriverIndex].driverName.c_str());

    sdl_renderer = SDL_CreateRenderer(sdl_window, sdlDriverIndex, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdl_renderer)
    {
        ADM_warning("[SDL] Failed to create an accelerated renderer, error: %s\n", SDL_GetError());
        sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
    }
    if (!sdl_renderer)
    {
        ADM_warning("[SDL] Failed to create a renderer, error: %s\n", SDL_GetError());
        cleanup();
        return false;
    }

    SDL_RendererInfo renderer_info;
    if (SDL_GetRendererInfo(sdl_renderer, &renderer_info) == 0)
    {
        ADM_info("[SDL] Using %s renderer\n", renderer_info.name);
        accel = ((renderer_info.flags & SDL_RENDERER_ACCELERATED) ? 1 : 0);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

    sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (sdl_texture)
    {
        useYV12 = true;
    }
    else
    {
        useYV12 = false;
        sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
        if (!sdl_texture)
        {
            ADM_warning("[SDL] FAILED to create a texture (rgb)\n");
            cleanup();
            return false;
        }
    }
    ADM_info("[SDL] Setting final size\n");
    changeZoom(zoom);
    ADM_info("[SDL] All init done.\n");
    return true;
}
/**
 *
 * @return
 */
bool sdlRenderImpl::cleanup()
{
    ADM_info("[SDL] Cleaning up\n");
    if (sdl_texture)
    {
        SDL_DestroyTexture(sdl_texture);
        sdl_texture = NULL;
    }
    if (sdl_renderer)
    {
        SDL_DestroyRenderer(sdl_renderer);
        sdl_renderer = NULL;
    }
    if (sdl_window)
    {
        // SDL_HideWindow(sdl_window);
        // buggy ....
#if defined(_WIN32) || defined(__APPLE__)
        SDL_DestroyWindow(sdl_window);
#endif
        // With x11, if we call it, it will cause a deadlock
        // if we dont, it will cause a crash at exit
        //
        sdl_window = NULL;
    }
    return true;
}
/**
    \fn displayImage
*/
bool sdlRenderImpl::displayImage(ADMImage *pic)
{
    if (!sdl_texture)
        return false;
    if (useYV12)
    {
        int imagePitch[3];
        uint8_t *imagePtr[3];
        pic->GetPitches(imagePitch);
        pic->GetWritePlanes(imagePtr);
        SDL_UpdateYUVTexture(sdl_texture, NULL, imagePtr[0], imagePitch[0], imagePtr[2], imagePitch[2], imagePtr[1],
                             imagePitch[1]);
    }
    else
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
    if (!sdl_texture)
        return false;
    SDL_RenderPresent(sdl_renderer);
    return true;
}
/**
    \fn changeZoom
*/
bool sdlRenderImpl::changeZoom(float newZoom)
{
    ADM_info("[SDL]changing zoom, sdl render.\n");
    calcDisplayFromZoom(newZoom);
    rescaleDisplay();
    currentZoom = newZoom;
    if (sdl_renderer)
    {
        float scaleX = (float)displayWidth / (float)imageWidth;
        float scaleY = (float)displayWidth / (float)imageHeight;

        SDL_RenderSetScale(sdl_renderer, scaleX, scaleY);
        SDL_SetWindowSize(sdl_window, displayWidth, displayHeight);
    }
    return true;
}
const char *sdlRenderImpl::getName()
{
    switch (accel)
    {
    case 0:
        return "SDL2-SW";
    case 1:
        return "SDL2-HW";
    default:
        break;
    }
    return "SDL2-??";
}

/**
    \fn initSdl
*/
bool initSdl(const std::string &sdlDriverName)
{
    ADM_info("[SDL] System-wide:  Initializing SDL\n");
    SDL_version version;
    SDL_version *ver = &version;

    SDL_GetVersion(ver);

    int sdl_version = (ver->major * 1000) + (ver->minor * 100) + (ver->patch);

    ADM_info("[SDL] Version: %u.%u.%u\n", ver->major, ver->minor, ver->patch);

    uint32_t sdlInitFlags;
    sdlInitFlags = SDL_INIT_AUDIO | SDL_INIT_VIDEO;

    if (SDL_Init(sdlInitFlags))
    {
        ADM_warning("[SDL] Init failed, error: %s\n", SDL_GetError());
        return false;
    }
    ADM_info("[SDL] Initialisation succeeded.\n");

    int nbDrivers = SDL_GetNumVideoDrivers();

    if (nbDrivers < 1)
    {
        ADM_warning("No SDL video drivers available.\n");
        return false;
    }

    for (int i = 0; i < nbDrivers; i++)
    {
        const char *driverName = SDL_GetVideoDriver(i);
        if (driverName)
            ADM_info("[SDL] Video driver %d / %d: %s\n", i, nbDrivers, driverName);
    }

    ADM_info("[SDL] Render drivers initialization\n");
    initDrivers();
    setSdlDriverByName(sdlDriverName);
    ADM_info("[SDL] initSDL done successfully.\n");
    return true;
}
/**
    \fn quitSdl
*/
void quitSdl(void)
{
    ADM_info("[SDL] quitSDL.\n");
    SDL_Quit();
}
#endif
