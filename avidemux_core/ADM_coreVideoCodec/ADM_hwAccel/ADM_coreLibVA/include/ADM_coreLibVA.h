/***************************************************************************
    \file             : ADM_coreLibva.h
    \brief            : Wrapper around libva functions
    \author           : (C) 2013 by mean fixounet@free.fr, derived from xbmc_pvr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_CORE_LIBVA_H
#define ADM_CORE_LIBVA_H

#ifdef USE_LIBVA
#include "ADM_windowInfo.h"
#include "X11/Xlib.h"
#include "va/va.h"
#include "ADM_image.h"
/**
    \class admXvba
*/
#define VA_INVALID -1
class admLibVA
{
protected:
    static GUI_WindowInfo      myWindowInfo;
public:
    static bool         init(GUI_WindowInfo *x);
    static bool         isOperationnal(void);
    static bool         cleanup(void);
    /* Surface */   
    
static  VAContextID createDecoder(int width, int height);
static  bool        destroyDecoder(VAContextID decoder);

static  VASurfaceID allocateSurface(void *session, int w, int h);
static  void        destroySurface(void *session, VASurfaceID surface);


static bool        transfer(void *session, int w, int h,void *surface, ADMImage *img,uint8_t *tmp);
 

};
#endif
#endif
