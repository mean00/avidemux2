/***************************************************************************
                          gui_render.cpp  -  description

    The final render a frame. The external interface is the same
    whatever the mean (RGB/YUV/Xv)
                             -------------------
    begin                : Thu Jan 3 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// clang-format off
#include "ADM_default.h"
#include "config.h"
#include "GUI_render.h"
#include "DIA_coreToolkit.h"
#include "GUI_accelRender.h"
#include "GUI_nullRender.h"
#include "GUI_renderInternal.h"
#include "prefs.h"
#include "ADM_qtx.h"
// clang-format on

#include "ADM_colorspace.h"
#include "DIA_uiTypes.h"

ADM_renderContext renderContext;
//_____________________________________
//_____________________________________
static VideoRenderBase *renderer = NULL;
static uint8_t *accelSurface = NULL;
static bool spawnRenderer(void);
//_______________________________________

static uint8_t _lock = 0;

static const UI_FUNCTIONS_T *HookFunc = NULL;
static bool enableDraw = true;

refreshSB refreshCallback = NULL;
/**
 * \fn renderGetName
 * @param name
 */
void renderGetName(std::string &name)
{
    if (!renderer)
        name = std::string("None");
    else
        name = std::string(renderer->getName());
}

/**
    \fn renderHookRefreshRequest
    \brief Hook the callback when a renderer requests a full redraw
*/
bool renderHookRefreshRequest(refreshSB cb)
{
    refreshCallback = cb;
    return true;
}
/**
 *      \fn ADM_renderLibInit
 *      \brief Initialize the renderlib with the needed external functions
 *
 */
uint8_t ADM_renderLibInit(const UI_FUNCTIONS_T *funcs)
{
    HookFunc = funcs;
    ADM_assert(funcs->apiVersion == ADM_RENDER_API_VERSION_NUMBER);
    return 1;
}
//**************************************
//**************************************
//**************************************
#define RENDER_CHECK(x)                                                                                                \
    {                                                                                                                  \
        ADM_assert(HookFunc);                                                                                          \
        ADM_assert(HookFunc->x);                                                                                       \
    }
static void MUI_getWindowInfo(void *draw, GUI_WindowInfo *xinfo)
{
    RENDER_CHECK(UI_getWindowInfo);
    HookFunc->UI_getWindowInfo(draw, xinfo);
}
static void MUI_updateDrawWindowSize(void *win, uint32_t w, uint32_t h)
{
    RENDER_CHECK(UI_updateDrawWindowSize);
    HookFunc->UI_updateDrawWindowSize(win, w, h);
}
static ADM_RENDER_TYPE MUI_getPreferredRender(void)
{
    RENDER_CHECK(UI_getPreferredRender);
    return HookFunc->UI_getPreferredRender();
}
void *MUI_getDrawWidget(void)
{
    RENDER_CHECK(UI_getDrawWidget);
    return HookFunc->UI_getDrawWidget();
}

//**************************************
//**************************************
//**************************************

/**
    Render init, initialize internals. Constuctor like function

*/
uint8_t renderInit(void)
{
    renderContext.draw = MUI_getDrawWidget();
    enableDraw = false;
    return 1;
}

void renderDestroy(void)
{
    ADM_info("Cleaning up Render\n");
    if (renderer)
    {
        renderer->stop();
        delete renderer;
        renderer = NULL;
    }
    enableDraw = false;
}

/**
    \fn renderLock
    \brief Take the weak lock (i.e. not threadsafe)
*/
uint8_t renderLock(void)
{
    ADM_assert(!_lock);
    _lock = 1;
    return 1;
}
uint8_t renderUnlock(void)
{
    ADM_assert(_lock);
    _lock = 0;
    return 1;
}

/**
    Warn the renderer that the display size is changing

*/
//----------------------------------------
uint8_t renderDisplayResize(uint32_t w, uint32_t h, float zoom)
{
    bool create = false;
    enableDraw = false;
    ADM_info("Render to %" PRIu32 "x%" PRIu32 " zoom=%.4f, old one =%d x %d, zoom=%.4f, renderer=%p\n", w, h, zoom,
             renderContext.phyW, renderContext.phyH, renderContext.lastZoom, renderer);
    // Check if something has changed...

    if (renderer && w == renderContext.phyW && h == renderContext.phyH && zoom == renderContext.lastZoom)
    {
        ADM_info("          No change, nothing to do\n");
        return true;
    }

    if (!renderer || (w != renderContext.phyW || h != renderContext.phyH))
    {
        if (renderer)
        {
            renderer->stop();
            delete renderer;
            renderer = NULL;
        }
        renderContext.phyW = w;
        renderContext.phyH = h;
        renderContext.lastZoom = zoom;
        if (w && h)
            spawnRenderer();
    }
    else // only zoom changed
    {
        renderer->changeZoom(zoom);
    }
    // Resize widget to be the same as input after zoom
    renderContext.lastZoom = zoom;
    MUI_updateDrawWindowSize(renderContext.draw, (uint32_t)((float)w * zoom), (uint32_t)((float)h * zoom));
    if (w && h)
        renderCompleteRedrawRequest();
    UI_purge();
    return 1;
}
/**
    Update the image and render it
    The width and hiehgt must NOT have changed

*/
//----------------------------------------
uint8_t renderUpdateImage(ADMImage *image)
{
    if (!renderer)
    {
        ADM_warning("Render update image without renderer\n");
        return 0;
    }
    ADM_assert(!_lock);
    enableDraw = true;
    if (renderer->getPreferedImage() != image->refType)
        image->hwDownloadFromRef();

    renderer->displayImage(image);
    return 1;
}
/**
    Refresh the image from internal buffer / last image
    Used for example as call back for X11 events

*/
//_______________________________________________
uint8_t renderRefresh(void)
{
    if (_lock)
        return 1;
    if (enableDraw == false)
        return true;
    if (renderer)
        renderer->refresh();
    return 1;
}
/**
    \fn renderCompleteRedrawRequest
    \brief ask the *caller* to redraw, whereas expose/refresh asks the renderer to refresh
*/
bool renderCompleteRedrawRequest(void)
{
    ADM_info("RedrawRequest\n");
    if (enableDraw == false)
        return true;
    if (refreshCallback)
        refreshCallback();
    return true;
}
/**
    \fn renderExpose
*/
uint8_t renderExpose(void)
{
    if (enableDraw == false)
        return true;
    renderRefresh();
    return 1;
}
/**                                                                                                                    \
 *                                                                                                                     \
 * @param renderName                                                                                                   \
 * @return                                                                                                             \
 */
extern VideoRenderBase *spawnDefaultRenderer(ADM_RENDER_TYPE preferred, ADM_renderContext &ctx);
extern VideoRenderBase *spawnCommonRenderer(ADM_RENDER_TYPE preferred, ADM_renderContext &ctx);

#ifdef __APPLE__
#elif defined _WIN32
VideoRenderBase *spawnWin32Renderer(ADM_RENDER_TYPE preferred, ADM_renderContext &ctx);
#else // linux
VideoRenderBase *spawnLinuxRenderer(ADM_RENDER_TYPE preferred, ADM_renderContext &ctx);
#endif

#define TRY_RENDERER(spawnFactory)                                                                                     \
    {                                                                                                                  \
        renderer = spawnFactory(prefRenderer, renderContext);                                                          \
        if (renderer)                                                                                                  \
            return true;                                                                                               \
    }

/**
 *
 *
 */
bool spawnRenderer(void)
{
    ADM_RENDER_TYPE prefRenderer = (ADM_RENDER_TYPE)MUI_getPreferredRender();
    bool r = false;

    GUI_WindowInfo xinfo;
    MUI_getWindowInfo(renderContext.draw, &renderContext.xinfo);

    // lookup renderer
    TRY_RENDERER(spawnCommonRenderer)
#ifdef __APPLE__

#elif defined _WIN32
    TRY_RENDERER(spawnWin32Renderer);
#else // linux
    TRY_RENDERER(spawnLinuxRenderer);
#endif
    // none found, use default
    renderer = spawnDefaultRenderer(RENDER_DEFAULT, renderContext);
    return true;
}

/**
    \fn
*/
uint8_t renderStartPlaying(void)
{

    return 1;
}

/**
    \fn renderStopPlaying
*/
uint8_t renderStopPlaying(void)
{

    return true;
}
/**
    \fn renderExposeEventFromUI
    \brief retrurn true if UI(gtk/qt) should handle redraw
*/
bool renderExposeEventFromUI(void)
{
    if (!renderer)
        return true;
    if (renderer->usingUIRedraw() == true)
        return true;
    renderer->refresh();
    return false;
}
/**
    \fn renderGetPreferedImageFormat
    \brief get the prefered hw accelerated image format (NONE,VDPAU,...)
*/
ADM_HW_IMAGE renderGetPreferedImageFormat(void)
{
    if (!renderer)
        return ADM_HW_NONE;
    return renderer->getPreferedImage();
}

//***************************************
/**
    \fn calcDisplayFromZoom
*/
bool VideoRenderBase::calcDisplayFromZoom(float zoom)
{
    displayWidth = (uint32_t)((float)imageWidth * zoom);
    displayHeight = (uint32_t)((float)imageHeight * zoom);
    return true;
}

/**
    \fn baseInit
*/
bool VideoRenderBase::baseInit(uint32_t w, uint32_t h, float zoom)
{
    imageWidth = w;
    imageHeight = h;
    currentZoom = zoom;
    calcDisplayFromZoom(zoom);
    return true;
}

/**
 * \fn renderClearInstance
 * \brief warn render that the renderer has been deleted by low level (window manager)
 * Would be prettier to do it with refCounting
 * @return
 */
bool renderClearInstance(void)
{
    renderer = NULL;
    return true;
}

// EOF
