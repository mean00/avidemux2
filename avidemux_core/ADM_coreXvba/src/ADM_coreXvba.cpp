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
#include "ADM_windowInfo.h"


GUI_WindowInfo      admXvba::myWindowInfo;

namespace ADM_coreXvba
{
 XvbaFunctions          funcs; 
 void                   *context;
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
    ADM_coreXvba::context=NULL;
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
  GetMe(             createDecodeBuffer,      XVBACreateDecodeBuffers)
  GetMe(             destroyDecodeBuffer,     XVBADestroyDecodeBuffers)
  GetMe(             getCapDecode,             XVBAGetCapDecode) 
  GetMe(             createDecode,             XVBACreateDecode)
  GetMe(             destroyDecode,            XVBADestroyDecode)
  GetMe(             startDecodePicture,       XVBAStartDecodePicture)
  GetMe(             decodePicture,            XVBADecodePicture)
  GetMe(             endDecodePicture,         XVBAEndDecodePicture)
  GetMe(             syncSurface,              XVBASyncSurface)
  GetMe(             getSurface,               XVBAGetSurface)
  GetMe(             transferSurface,          XVBATransferSurface)
    
  // Time to query
   int version=0;
   if(!ADM_coreXvba::funcs.queryExtension((Display *)x->display,&version))
   {
       ADM_warning("Xvba Query extension failed\n");
       return false;
   }
  ADM_info("Xvba version %d\n",version);
  // Create global context
#define CLEAR(x) memset(&x,0,sizeof(x))
  XVBA_Create_Context_Input  contextInput;
  XVBA_Create_Context_Output contextOutput;
  CLEAR(contextInput);
  CLEAR(contextOutput);
  
  contextInput.display=(Display *)x->display;
  contextInput.draw=(Drawable )x->widget; // fixme
  contextInput.size=sizeof(contextInput);
  
  contextOutput.size=sizeof(contextOutput);
  
  if(Success!=ADM_coreXvba::funcs.createContext(&contextInput,&contextOutput))
  {
      ADM_warning("Xvba context creation failed\n");
      return false;
  }
    ADM_coreXvba::context=contextOutput.context;
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
           if(ADM_coreXvba::context)
           {
               ADM_coreXvba::funcs.destroyContext(ADM_coreXvba::context);
               ADM_coreXvba::context=NULL;
           }
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
