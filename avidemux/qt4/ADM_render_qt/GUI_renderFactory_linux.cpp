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
#ifdef USE_XV
extern VideoRenderBase *spawnXvRender();
#endif

#ifdef USE_VDPAU
extern VideoRenderBase *spawnVDPAURender();
#endif

#ifdef USE_LIBVA
extern VideoRenderBase *spawnLIBVARender();
#endif

/**                                                                                                                    \
 *                                                                                                                     \
 * @param renderName                                                                                                   \
 * @return                                                                                                             \
 */
VideoRenderBase *spawnLinuxRenderer(ADM_RENDER_TYPE preferred, ADM_renderContext &ctx)
{
    bool r;
    switch (preferred)
    {

#if defined(USE_VDPAU)
    case RENDER_VDPAU:
        TRY_RENDERER_QT(spawnVDPAURender, "VDPAU")
        break;
#endif
#if defined(USE_LIBVA)
    case RENDER_LIBVA:
        TRY_RENDERER_QT(spawnLIBVARender, "LIBVA")
        break;
#endif

#if defined(USE_XV)
    case RENDER_XV:
        TRY_RENDERER_QT(spawnXvRender, "Xv")
        break;
#endif

    default:
        break;
    }
    return NULL;
}
// EOF
