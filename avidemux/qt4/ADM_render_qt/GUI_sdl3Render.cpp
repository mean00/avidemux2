/***************************************************************************
    copyright            : (C) 2026
    email                : fixounet@free.fr
 *
 * SDL3 version
***************************************************************************/

#include "ADM_default.h"
#include "config.h"
//

// clang-format off
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ADM_qtx.h"

extern "C"
{
// get rid of warnings due to different definitions
#undef HAVE_INTTYPES_H
#undef HAVE_MALLOC_H
#undef HAVE_STDINT_H
#undef HAVE_SYS_TYPES_H
#include <SDL3/SDL.h>
}


#include "ADM_colorspace.h"
#include "GUI_render.h"
#include "GUI_accelRender.h"
#include "GUI_sdl3Render.h"
// clang-format on

class sdl3RenderImpl : public VideoRenderBase
{
  protected:
    GUI_WindowInfo winfo;
    int accel;

  public:
    sdl3RenderImpl(void);
    virtual ~sdl3RenderImpl();
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
 * spawnSdl3Render
 */
VideoRenderBase *spawnSdl3Render()
{
    return new sdl3RenderImpl();
}

/**
 * static SDL logging
 */
static void SDL_Logger(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    ADM_info("[SDL3] %s\n", message);
}

/**
 * sdlRenderImpl
 */
sdl3RenderImpl::sdl3RenderImpl(void)
{
    sdl_running = false;
    sdl_window = NULL;
    sdl_renderer = NULL;
    sdl_texture = NULL;
    accel = -1;
    memset(&winfo, 0, sizeof(GUI_WindowInfo));
    winfo.scalingFactor = 1.;
    ADM_info("[SDL3] Created\n");
}

sdl3RenderImpl::~sdl3RenderImpl()
{
    stop();
}

bool sdl3RenderImpl::stop(void)
{
    ADM_info("[SDL3] Stopping\n");
    cleanup();
    if (sdl_running)
    {
        ADM_info("[SDL3] Video subsystem closed\n");
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        sdl_running = false;
    }
    return true;
}

void sdl3RenderImpl::rescaleDisplay(void)
{
    double dw = displayWidth;
    double dh = displayHeight;
    dw *= winfo.scalingFactor;
    dh *= winfo.scalingFactor;
    displayWidth = (uint32_t)(dw + 0.5);
    displayHeight = (uint32_t)(dh + 0.5);
}

bool sdl3RenderImpl::init(GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom)
{
    ADM_info("[SDL3] Initializing video subsystem\n");

    winfo = *window;
    baseInit(w, h, zoom);
    rescaleDisplay();

    SDL_SetLogOutputFunction(SDL_Logger, NULL);

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
        ADM_warning("[SDL3] Video subsystem init failed, error: %s\n", SDL_GetError());
        return false;
    }
    ADM_info("[SDL3] Video subsystem init ok\n");

    sdl_running = true;

    if (admDetectQtEngine() == QT_WAYLAND_ENGINE)
    {
        ADM_info("[SDL3] Using Wayland backend for borderless window\n");
        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "avidemux_sdl3");
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, winfo.x);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, winfo.y);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, displayWidth);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, displayHeight);
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true);

        sdl_window = SDL_CreateWindowWithProperties(props);
        SDL_DestroyProperties(props);
    }
    else
    {
        ADM_info("[SDL3] Using X11 backend - wrapping existing handle %lx\n", (unsigned long)window->systemWindowId);
        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER, (Sint64)window->systemWindowId);
        sdl_window = SDL_CreateWindowWithProperties(props);
        SDL_DestroyProperties(props);
    }

    if (!sdl_window)
    {
        ADM_warning("[SDL3] Creating window failed, error: %s\n", SDL_GetError());
        cleanup();
        return false;
    }

    ADM_info("[SDL3] Creating renderer\n");
    sdl_renderer = SDL_CreateRenderer(sdl_window, NULL);
    if (!sdl_renderer)
    {
        ADM_warning("[SDL3] Failed to create a renderer, error: %s\n", SDL_GetError());
        cleanup();
        return false;
    }

    SDL_SetRenderScale(sdl_renderer, 1.0f, 1.0f);

    sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (!sdl_texture)
    {
        ADM_warning("[SDL3] FAILED to create a texture (YV12)\n");
        cleanup();
        return false;
    }

    ADM_info("[SDL3] Setting final size\n");
    changeZoom(zoom);
    ADM_info("[SDL3] All init done.\n");
    return true;
}

bool sdl3RenderImpl::cleanup()
{
    ADM_info("[SDL3] Cleaning up\n");
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
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
    }
    return true;
}

bool sdl3RenderImpl::displayImage(ADMImage *pic)
{
    if (!sdl_texture)
        return false;

    int imagePitch[3];
    uint8_t *imagePtr[3];
    pic->GetPitches(imagePitch);
    pic->GetWritePlanes(imagePtr);

    SDL_UpdateYUVTexture(sdl_texture, NULL, imagePtr[0], imagePitch[0], imagePtr[2], imagePitch[2], imagePtr[1],
                         imagePitch[1]);

    SDL_RenderClear(sdl_renderer);
    SDL_RenderTexture(sdl_renderer, sdl_texture, NULL, NULL);
    refresh();
    return true;
}

bool sdl3RenderImpl::refresh(void)
{
    if (!sdl_texture)
        return false;
    SDL_RenderPresent(sdl_renderer);
    return true;
}

bool sdl3RenderImpl::changeZoom(float newZoom)
{
    ADM_info("[SDL3] changing zoom.\n");
    calcDisplayFromZoom(newZoom);
    rescaleDisplay();
    currentZoom = newZoom;
    if (sdl_renderer)
    {
        float scaleX = (float)displayWidth / (float)imageWidth;
        float scaleY = (float)displayWidth / (float)imageHeight;

        SDL_SetRenderScale(sdl_renderer, scaleX, scaleY);
        SDL_SetWindowSize(sdl_window, displayWidth, displayHeight);
    }
    return true;
}

const char *sdl3RenderImpl::getName()
{
    return "SDL 3.0 HW";
}
