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


#include "ADM_image.h"
typedef bool (*refreshSB)(void);
#include "ADM_windowInfo.h"

typedef enum 
{
        ZOOM_1_4,
        ZOOM_1_2,
        ZOOM_1_1,
        ZOOM_2,
        ZOOM_4,
        ZOOM_INVALID
}renderZoom;

uint8_t renderInit( void );
void    renderDestroy(void);
uint8_t renderDisplayResize(uint32_t w, uint32_t h,renderZoom zoom);
uint8_t renderRefresh(void);
uint8_t renderExpose(void);
uint8_t renderUpdateImage(ADMImage *img);
uint8_t renderUpdateImageBlit(uint8_t *ptr,uint32_t startx, uint32_t starty, uint32_t w, uint32_t,uint32_t primary);
bool    renderCompleteRedrawRequest(void); // will call admPreview 
uint8_t renderStartPlaying( void );
uint8_t renderStopPlaying( void );
bool    renderExposeEventFromUI(void); // This is called by UI, return true if UI should redraw, false else

uint8_t renderLock(void);
uint8_t renderUnlock(void);
bool renderHookRefreshRequest(refreshSB cb);

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

#ifdef __WIN32
		RENDER_DIRECTX=3,
#endif
#endif
#ifdef USE_VDPAU
        RENDER_VDPAU=4,

#endif
#ifdef USE_OPENGL
        RENDER_QTOPENGL=5,
#endif

        RENDER_LAST       


}ADM_RENDER_TYPE;

#endif
