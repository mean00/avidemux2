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
#include "GUI_renderFactory.h"
#include "prefs.h"

extern VideoRenderBase *spawnSimpleRender();

#ifdef USE_SDL
extern VideoRenderBase *spawnSdlRender();
#endif

#if defined(USE_OPENGL)
extern VideoRenderBase *RenderSpawnQtGl(void);
#endif
/**                                                                                                                    \
 *                                                                                                                     \
 *                                                                                                                     \
 *                                                                                                                     \
 */
VideoRenderBase *spawnDefaultRenderer(ADM_RENDER_TYPE preferred, ADM_renderContext &ctx)
{
    bool r;
    TRY_RENDERER_SPAWN_ALL(spawnSimpleRender, "simpleRenderer");
    ADM_assert(0);
    return NULL;
}

/**                                                                                                                    \
 *                                                                                                                     \
 * @param renderName                                                                                                   \
 * @return                                                                                                             \
 */
VideoRenderBase *spawnCommonRenderer(ADM_RENDER_TYPE preferred, ADM_renderContext &ctx)
{
    bool r;
    switch (preferred)
    {
#if defined(USE_OPENGL)
    case RENDER_QTOPENGL: {
        bool hasOpenGl = false;
        prefs->get(FEATURES_ENABLE_OPENGL, &hasOpenGl);
        if (!hasOpenGl)
        {
            ADM_warning("OpenGl is disabled\n");
        }
        else
        {
            TRY_RENDERER_SPAWN_ALL(RenderSpawnQtGl, "QtGl");
        }
        break;
    }
#endif

#if defined(USE_SDL)
    case RENDER_SDL:
        TRY_RENDERER_SPAWN_ALL(spawnSdlRender, "SDL")
        break;
#endif
    default:
        break;
    }
    return NULL;
}
// EOF
