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

typedef Bool        (*XVBAQueryExtensionProc)       (Display *dpy, int *vers);
typedef Status      (*XVBACreateContextProc)        (void *input, void *output);
typedef Status      (*XVBADestroyContextProc)       (void *context);
typedef Bool        (*XVBAGetSessionInfoProc)       (void *input, void *output);
typedef Status      (*XVBACreateSurfaceProc)        (void *input, void *output);
typedef Status      (*XVBACreateGLSharedSurfaceProc)(void *input, void *output);
typedef Status      (*XVBADestroySurfaceProc)       (void *surface);
typedef Status      (*XVBACreateDecodeBuffersProc)  (void *input, void *output);
typedef Status      (*XVBADestroyDecodeBuffersProc) (void *input);
typedef Status      (*XVBAGetCapDecodeProc)         (void *input, void *output);
typedef Status      (*XVBACreateDecodeProc)         (void *input, void *output);
typedef Status      (*XVBADestroyDecodeProc)        (void *session);
typedef Status      (*XVBAStartDecodePictureProc)   (void *input);
typedef Status      (*XVBADecodePictureProc)        (void *input);
typedef Status      (*XVBAEndDecodePictureProc)     (void *input);
typedef Status      (*XVBASyncSurfaceProc)          (void *input, void *output);
typedef Status      (*XVBAGetSurfaceProc)           (void *input);
typedef Status      (*XVBATransferSurfaceProc)      (void *input);


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
  XVBACreateDecodeBuffersProc         *createDecodeBuffers;
  XVBADestroyDecodeBuffersProc        *destroyDecodeBuffers;
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

namespace ADM_coreXvba
{
 extern XvbaFunctions         funcs;
 extern void                  *xvbDevice;
}

#define CHECK(x) if(!isOperationnal()) {ADM_error("xvba is not operationnal\n");return VDP_STATUS_ERROR;}\
                 VdpStatus r=x;\
                 if(VDP_STATUS_OK!=r) {ADM_warning(#x" call failed with error=%s\n",getErrorString(r));}return r;

#define CHECKBOOL(x) if(!isOperationnal())\
                    {ADM_error("xvba is not operationnal\n");return false;}\
                 VdpStatus r=x;\
                 if(VDP_STATUS_OK!=r)  \
                    {\
                    ADM_warning(#x" call failed with error=%s\n",getErrorString(r));\
                    return false;};

#endif