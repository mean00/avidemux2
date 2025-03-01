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
#if defined(USE_DXVA2)
extern VideoRenderBase *RenderSpawnDxva2(void);
#endif

/**                                                                                                                    \
 *                                                                                                                     \
 * @param renderName                                                                                                   \
 * @return                                                                                                             \
 */
VideoRenderBase *spawnWin32Renderer(ADM_RENDER_TYPE preferred, ADM_renderContext &ctx)
{
    bool r;
    switch (preferred)
    {
#if defined(USE_DXVA2)
    case RENDER_DXVA2: {
        TRY_RENDERER_SPAWN_ALL(RenderSpawnDxva2, "Dxva2");
        break;
    }
#endif
    default:
        break;
    }
    return NULL;
}
// EOF
