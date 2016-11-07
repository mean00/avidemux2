/** **************************************************************************
     \file GUI_render.h
     \brief Lib to draw stuff on screen.
    copyright            : (C) 2001--2008 by mean
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


#ifndef GUI_RENDER_H
#define GUI_RENDER_H

#include "ADM_render6_export.h"
#include "ADM_image.h"
typedef bool (*refreshSB)(void);
#include "ADM_windowInfo.h"
#include <string>

typedef enum 
{
        ZOOM_1_4,
        ZOOM_1_2,
        ZOOM_1_1,
        ZOOM_2,
        ZOOM_4,
        ZOOM_AUTO,
        ZOOM_INVALID
}renderZoom;

ADM_RENDER6_EXPORT uint8_t renderInit( void );
ADM_RENDER6_EXPORT void    renderDestroy(void);
ADM_RENDER6_EXPORT uint8_t renderDisplayResize(uint32_t w, uint32_t h,renderZoom zoom);
uint8_t renderRefresh(void);
ADM_RENDER6_EXPORT uint8_t renderExpose(void);
ADM_RENDER6_EXPORT uint8_t renderUpdateImage(ADMImage *img);
uint8_t renderUpdateImageBlit(uint8_t *ptr,uint32_t startx, uint32_t starty, uint32_t w, uint32_t,uint32_t primary);
bool    renderCompleteRedrawRequest(void); // will call admPreview 
ADM_RENDER6_EXPORT uint8_t renderStartPlaying( void );
ADM_RENDER6_EXPORT uint8_t renderStopPlaying( void );
ADM_RENDER6_EXPORT bool    renderExposeEventFromUI(void); // This is called by UI, return true if UI should redraw, false else
ADM_RENDER6_EXPORT ADM_HW_IMAGE renderGetPreferedImageFormat(void);
ADM_RENDER6_EXPORT uint8_t renderLock(void);
ADM_RENDER6_EXPORT uint8_t renderUnlock(void);
ADM_RENDER6_EXPORT bool    renderHookRefreshRequest(refreshSB cb);
ADM_RENDER6_EXPORT void    renderGetName(std::string &name);
ADM_RENDER6_EXPORT bool    renderClearInstance(void);


/* These functions are trampolined through render as they are UI dependant */

void *UI_getDrawWidget(void);
void UI_rgbDraw(void *widg,uint32_t w, uint32_t h,uint8_t *ptr);
void UI_updateDrawWindowSize(void *win,uint32_t w,uint32_t h);
void UI_getWindowInfo(void *draw, GUI_WindowInfo *xinfo);

/* The list of render engine we support. Warning the list is UI dependant, i.e. for example on macOsX, the GTK version can do Xv, but the QT4 one cannot */
typedef enum 
{
        RENDER_GTK=0,
#ifdef USE_XV
        RENDER_XV=1,
#endif

#ifdef USE_SDL
        RENDER_SDL=2,

#ifdef _WIN32
		RENDER_DIRECTX=3,
#endif
#endif
#ifdef USE_VDPAU
        RENDER_VDPAU=4,

#endif
#ifdef USE_OPENGL
        RENDER_QTOPENGL=5,
#endif
#ifdef USE_LIBVA
        RENDER_LIBVA=6,
#endif
#ifdef USE_DXVA2,
        RENDER_DXVA2=6,
#endif USE_DXVA2,

        RENDER_LAST       


}ADM_RENDER_TYPE;

#endif
