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

#pragma once

#include "ADM_image.h"
#include "ADM_render6_export.h"
typedef bool (*refreshSB)(void);
#include "ADM_windowInfo.h"
#include <string>

#define ZOOM_AUTO -1.
#define ZOOM_1_4 0.25
#define ZOOM_1_2 0.5
#define ZOOM_1_1 1.

ADM_RENDER6_EXPORT uint8_t renderInit(void);
ADM_RENDER6_EXPORT void renderDestroy(void);
ADM_RENDER6_EXPORT uint8_t renderDisplayResize(uint32_t w, uint32_t h, float zoom);
uint8_t renderRefresh(void);
ADM_RENDER6_EXPORT uint8_t renderExpose(void);
ADM_RENDER6_EXPORT uint8_t renderUpdateImage(ADMImage *img);
uint8_t renderUpdateImageBlit(uint8_t *ptr, uint32_t startx, uint32_t starty, uint32_t w, uint32_t, uint32_t primary);
bool renderCompleteRedrawRequest(void); // will call admPreview
ADM_RENDER6_EXPORT uint8_t renderStartPlaying(void);
ADM_RENDER6_EXPORT uint8_t renderStopPlaying(void);
ADM_RENDER6_EXPORT bool renderExposeEventFromUI(
    void); // This is called by UI, return true if UI should redraw, false else
ADM_RENDER6_EXPORT ADM_HW_IMAGE renderGetPreferedImageFormat(void);
ADM_RENDER6_EXPORT uint8_t renderLock(void);
ADM_RENDER6_EXPORT uint8_t renderUnlock(void);
ADM_RENDER6_EXPORT bool renderHookRefreshRequest(refreshSB cb);
ADM_RENDER6_EXPORT void renderGetName(std::string &name);
ADM_RENDER6_EXPORT bool renderClearInstance(void);

/* These functions are trampolined through render as they are UI dependant */

void *UI_getDrawWidget(void);
void UI_rgbDraw(void *widg, uint32_t w, uint32_t h, uint8_t *ptr);
void UI_updateDrawWindowSize(void *win, uint32_t w, uint32_t h);
void UI_getWindowInfo(void *draw, GUI_WindowInfo *xinfo);
void UI_resize(uint32_t width, uint32_t height);
bool UI_getNeedsResizingFlag(void);
void UI_setNeedsResizingFlag(bool resize);
void UI_setBlockZoomChangesFlag(bool block);
void UI_resetZoomThreshold(void);
void UI_setZoomToFitIntoWindow(void);
void UI_displayZoomLevel(void);
void UI_getMaximumPreviewSize(uint32_t *availWidth, uint32_t *availHeight);

/* The list of render engine we support. Warning the list is UI dependant, i.e. for example on macOsX, the GTK version
 * can do Xv, but the QT4 one cannot */
typedef enum
{
    RENDER_XV = 1,
    RENDER_SDL = 2,
    RENDER_DIRECTX = 3,
    RENDER_VDPAU = 4,
    RENDER_QTOPENGL = 5,
    RENDER_LIBVA = 7,
    RENDER_DXVA2 = 8,
    RENDER_DEFAULT = 9,
    RENDER_GTK = 10,
    RENDER_LAST
} ADM_RENDER_TYPE;
/**
 *
 */
typedef struct
{
    int phyW;
    int phyH;
    float lastZoom;
    GUI_WindowInfo xinfo;
    void *draw;
} ADM_renderContext;
