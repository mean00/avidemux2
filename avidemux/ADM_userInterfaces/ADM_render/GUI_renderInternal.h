/****************************************************************************
 copyright            : (C) 2001 by mean
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


#ifndef GUI_RENDER_INTERNAL_H
#define GUI_RENDER_INTERNAL_H

#define ADM_RENDER_API_VERSION_NUMBER 2
#include "ADM_userInterfaces/ADM_render/GUI_render.h"
typedef struct
{
  int   apiVersion;
  void (*UI_purge)(void);
  void (*UI_getWindowInfo)(void *draw, GUI_WindowInfo *xinfo);
  void (*UI_updateDrawWindowSize)(void *win,uint32_t w,uint32_t h);
  void (*UI_rgbDraw)(void *widg,uint32_t w, uint32_t h,uint8_t *ptr);
  void *(*UI_getDrawWidget)(void);
  ADM_RENDER_TYPE (*UI_getPreferredRender)(void);
  
}UI_FUNCTIONS_T;

uint8_t ADM_renderLibInit(const UI_FUNCTIONS_T *funcs);
#endif
