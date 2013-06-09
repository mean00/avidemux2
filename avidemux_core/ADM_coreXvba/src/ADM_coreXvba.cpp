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
#include "ADM_default.h"
#include "../include/ADM_coreXvba.h"



#ifdef USE_XVBA
#include "../include/ADM_coreXvbaInternal.h"
#include "ADM_dynamicLoading.h"


GUI_WindowInfo      admXvba::myWindowInfo;

namespace ADM_coreXvba
{
 XvbaFunctions          funcs; 
}

static ADM_LibWrapper        xvbaDynaLoader;
static bool                  coreXvbaWorking=false;



/**
    \fn     init
    \brief
*/
bool admXvba::init(GUI_WindowInfo *x)
{
    ADM_info("Loading Xvba library ...\n");
    memset(&ADM_coreXvba::funcs,0,sizeof(ADM_coreXvba::funcs));
    if(false==xvbaDynaLoader.loadLibrary("libXvBAW.so.1"))
    {
        ADM_info("Cannot load libxvba.so\n");
        return false;
    }

#define GetMe(fun,id)         {ADM_coreXvba::funcs.fun= (typeof( ADM_coreXvba::funcs.fun))xvbaDynaLoader.getSymbol(#id);\
                                if(! ADM_coreXvba::funcs.fun) {ADM_error("Cannot load symbol %s\n",#id);return false;}}
        
  GetMe(             queryExtension,           XVBAQueryExtension)
  GetMe(             createContext,            XVBACreateContext)
  GetMe(             destroyContext,           XVBADestroyContext)
  GetMe(             getSessionInfo,           XVBAGetSessionInfo)
  GetMe(             createSurface,            XVBACreateSurface)
  GetMe(             createGLSharedSurface,    XVBACreateGLSharedSurface)
  GetMe(             destroySurface,           XVBADestroySurface)
  GetMe(             createDecodeBuffers,      XVBACreateDecodeBuffers)
  GetMe(             destroyDecodeBuffers,     XVBADestroyDecodeBuffers)
  GetMe(             getCapDecode,             XVBAGetCapDecode) 
  GetMe(             createDecode,             XVBACreateDecode)
  GetMe(             destroyDecode,            XVBADestroyDecode)
  GetMe(             startDecodePicture,       XVBAStartDecodePicture)
  GetMe(             decodePicture,            XVBADecodePicture)
  GetMe(             endDecodePicture,         XVBAEndDecodePicture)
  GetMe(             syncSurface,              XVBASyncSurface)
  GetMe(             getSurface,               XVBAGetSurface)
  GetMe(             transferSurface,          XVBATransferSurface)
    
   
    coreXvbaWorking=true;
    myWindowInfo=*x;

    ADM_info("Xvba  init ok.\n");
    return true;
}
/**
    \fn cleanup
*/
bool admXvba::cleanup(void)
{
    if(true==coreXvbaWorking)
    {
           
    }
    coreXvbaWorking=false;
    return true;
}
/**
    \fn isOperationnal
*/
bool admXvba::isOperationnal(void)
{
    return coreXvbaWorking;
}

#else 
//******************************************
//******************************************
// Dummy when xvba is not there...
// Dummy when xvba is not there...
//******************************************
//******************************************
static bool                  coreVdpWorking=false;
bool admXvba::init(GUI_WindowInfo *x)
{
          return false;
}
  
/**
    \fn isOperationnal
*/
bool admXvba::isOperationnal(void)
{
    ADM_warning("This binary has no VPDAU support\n");
    return coreVdpWorking;
}
bool admXvba::cleanup(void)
{
    ADM_warning("This binary has no VPDAU support\n");
    return true;
}
#endif
// EOF
