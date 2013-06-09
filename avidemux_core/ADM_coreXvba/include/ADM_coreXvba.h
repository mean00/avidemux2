/***************************************************************************
    \file             : ADM_coreXvba.cpp
    \brief            : Wrapper around xvba functions
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

#ifndef ADM_CORE_XVBA_H
#define ADM_CORE_XVBA_H

#ifdef USE_XVBA
#include "ADM_windowInfo.h"
#include "X11/Xlib.h"
#include "amd/amdxvba.h"
/**
    \class admXvba
*/
class admXvba
{
protected:
    static GUI_WindowInfo      myWindowInfo;
public:
    static bool         init(GUI_WindowInfo *x);
    static bool         isOperationnal(void);
    static bool         cleanup(void);
    /* Surface */
#ifdef USE_XVBA
static  bool        queryExtensionProc       (Display *dpy, int *vers);
static  int         createContextProc        (void *input, void *output);
static  int         destroyContextProc       (void *context);
static  bool        getSessionInfoProc       (void *input, void *output);
static  int         createSurfaceProc        (void *input, void *output);
static  int         createGLSharedSurfaceProc(void *input, void *output);
static  int         destroySurfaceProc       (void *surface);
static  int         createDecodeBuffersProc  (void *input, void *output);
static  int         destroyDecodeBuffersProc (void *input);
static  int         getCapDecodeProc         (void *input, void *output);
static  int         createDecodeProc         (void *input, void *output);
static  int         destroyDecodeProc        (void *session);
static  int         startDecodePictureProc   (void *input);
static  int         decodePictureProc        (void *input);
static  int         syncSurfaceProc          (void *input, void *output);
static  int         getSurfaceProc           (void *input);
static  int         transferSurfaceProc      (void *input);


#endif
};
#endif
#endif