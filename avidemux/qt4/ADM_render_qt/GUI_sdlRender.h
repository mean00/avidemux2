
/***************************************************************************
    copyright            : (C) 2006 by mean
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
#include "ADM_render6_export.h"
#include "GUI_accelRender.h"
#include "GUI_render.h"
#include <string>
#include <vector>

/**
 */
typedef struct
{
    int index;
    int flags;
    std::string driverName;
} sdlDriverInfo;

/**
    \class sdlRender
*/
class sdlRender : public VideoRenderBase
{
  protected:
    bool useYV12;

  public:
    sdlRender(void);
    virtual ~sdlRender();
    virtual bool init(GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom);
    virtual bool stop(void);
    virtual bool displayImage(ADMImage *pic);
    virtual bool changeZoom(float newZoom);
    virtual bool usingUIRedraw(void);
    virtual bool refresh(void);
    const char *getName();

  protected:
    void *impl;

  protected:
};
// #define SDL_RENDER
#define SDL_RENDER ADM_RENDER6_EXPORT
SDL_RENDER const std::vector<sdlDriverInfo> &getListOfSdlDrivers();
SDL_RENDER bool setSdlDriverByName(const std::string &name);
SDL_RENDER std::string getSdlDriverName();
SDL_RENDER bool initSdl(const std::string &videoDevice);
SDL_RENDER void quitSdl(void);
