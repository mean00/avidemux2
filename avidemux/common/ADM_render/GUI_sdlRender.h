
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
#include "GUI_render.h"
#include "GUI_accelRender.h"
#include "ADM_render6_export.h"
#include <string>
#include <vector>

/**
 */
typedef struct 
{    
    int index;
    int flags;
    std::string driverName;
}sdlDriverInfo;


/**
    \class sdlRender
*/
class sdlRender: public VideoRenderBase
{
  protected:
              bool     useYV12;
  public:
                             sdlRender( void ) ;
              virtual	bool init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom);
              virtual	bool stop(void);				
              virtual   bool displayImage(ADMImage *pic);
              virtual   bool changeZoom(renderZoom newZoom);
              virtual   bool usingUIRedraw(void);
              virtual   bool refresh(void) ;              
            
protected:
};
ADM_RENDER6_EXPORT const std::vector<sdlDriverInfo> &getListOfSdlDrivers();
ADM_RENDER6_EXPORT bool  setSdlDriverByName(const std::string &name);
ADM_RENDER6_EXPORT std::string  getSdlDriverName();
ADM_RENDER6_EXPORT bool initSdl(const std::string  &videoDevice);
ADM_RENDER6_EXPORT void quitSdl(void);
