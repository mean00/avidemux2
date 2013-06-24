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
#include "ADM_image.h"
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
    
static  void        *createDecoder(int width, int height);
static  bool        destroyDecoder(void *decoder);

static  void        *allocateSurface(void *session, int w, int h);
static  void        destroySurface(void *session, void *surface);

//  type : XVBA_NONE = 0,    XVBA_PICTURE_DESCRIPTION_BUFFER,    XVBA_DATA_BUFFER,    
//  XVBA_DATA_CTRL_BUFFER,    XVBA_QM_BUFFER
static  XVBABufferDescriptor        *createDecodeBuffer(void *session,XVBA_BUFFER type);
static  void        destroyDecodeBuffer(void *session,XVBABufferDescriptor *buffer);

static bool        decodeStart(void *session, void *surface);
static bool        decode(void *session,void *picture_desc, void *matrix_desc,bool set,int off0,int size1);
static bool        decodeEnd(void *session);

static bool        transfer(void *session, int w, int h,void *surface, ADMImage *img,uint8_t *tmp);
 
static bool        syncSurface(void *session, void *surface, bool *ready);

//--
#if 0
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

#endif
};
#endif
#endif
