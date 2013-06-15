/**
 * ported from xbmc_pvr
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_CORE_XVBA_INTERNAL_H
#define ADM_CORE_XVBA_INTERNAL_H

// From xbmc 

typedef Bool        (XVBAQueryExtensionProc)       (Display *dpy, int *vers);
typedef Status      (XVBACreateContextProc)        (void *input, void *output);
typedef Status      (XVBADestroyContextProc)       (void *context);
typedef Bool        (XVBAGetSessionInfoProc)       (void *input, void *output);
typedef Status      (XVBACreateSurfaceProc)        (void *input, void *output);
typedef Status      (XVBACreateGLSharedSurfaceProc)(void *input, void *output);
typedef Status      (XVBADestroySurfaceProc)       (void *surface);
typedef Status      (XVBACreateDecodeBuffersProc)  (void *input);
typedef Status      (XVBADestroyDecodeBuffersProc) (void *input, void *output);
typedef Status      (XVBAGetCapDecodeProc)         ( XVBA_GetCapDecode_Input     *decodecap_list_input,   XVBA_GetCapDecode_Output    *decodecap_list_output);
typedef Status      (XVBACreateDecodeProc)         (void *input, void *output);
typedef Status      (XVBADestroyDecodeProc)        (void *session);
typedef Status      (XVBAStartDecodePictureProc)   (void *input);
typedef Status      (XVBADecodePictureProc)        (void *input);
typedef Status      (XVBAEndDecodePictureProc)     (void *input);
typedef Status      (XVBASyncSurfaceProc)          (void *input, void *output);
typedef Status      (XVBAGetSurfaceProc)           (void *input);
typedef Status      (XVBATransferSurfaceProc)      (void *input);


/**
    \fn VdpFunctions
    
*/
typedef struct 
{
    
  XVBAQueryExtensionProc              *queryExtension;
  XVBACreateContextProc               *createContext;
  XVBADestroyContextProc              *destroyContext;
  XVBAGetSessionInfoProc              *getSessionInfo;
  XVBACreateSurfaceProc               *createSurface;
  XVBACreateGLSharedSurfaceProc       *createGLSharedSurface;
  XVBADestroySurfaceProc              *destroySurface;
  XVBACreateDecodeBuffersProc         *createDecodeBuffer;
  XVBADestroyDecodeBuffersProc        *destroyDecodeBuffer;
  XVBAGetCapDecodeProc                *getCapDecode;
  XVBACreateDecodeProc                *createDecode;
  XVBADestroyDecodeProc               *destroyDecode;
  XVBAStartDecodePictureProc          *startDecodePicture;
  XVBADecodePictureProc               *decodePicture;
  XVBAEndDecodePictureProc            *endDecodePicture;
  XVBASyncSurfaceProc                 *syncSurface;
  XVBAGetSurfaceProc                  *getSurface;
  XVBATransferSurfaceProc             *transferSurface;
}XvbaFunctions;

#endif