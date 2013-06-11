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

#include <X11/Xlib.h>
#include <X11/Xutil.h>


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

#define CLEAR(x) memset(&x,0,sizeof(x))
#define PREPARE_IN(z) {CLEAR(z);z.size=sizeof(z);z.context=ADM_coreXvba::context;}
#define PREPARE_OUT(z) {CLEAR(z);z.size=sizeof(z);}
#define CHECK_ERROR(x) {xError=x;displayXError(#x,dis,xError);}

/**
 * \fn displayXError
 * @param dis
 * @param er
 */
static void displayXError(const char *func,Display *dis,int er)
{
    if(er==Success) return;
    char errString[200];
    XGetErrorText (dis,er,errString,sizeof(errString)-1);
    ADM_warning("X11 Error : <%s:%s>\n",func,errString);

}

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
    int xError;
    Display *dis=(Display *)x->display;
    
    
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
  
  XVBA_Create_Context_Input  contextInput;
  XVBA_Create_Context_Output contextOutput;
  CLEAR(contextInput);
  PREPARE_OUT(contextOutput);
  
  
  contextInput.display=dis;
  contextInput.draw= DefaultRootWindow(dis); // fixme
  
  CHECK_ERROR(ADM_coreXvba::funcs.createContext(&contextInput,&contextOutput))
  if(Success!=xError)
  {
      ADM_warning("Xvba context creation failed\n");
      return false;
  }

    // Get decode cap
    XVBA_GetCapDecode_Input  capin;
    XVBA_GetCapDecode_Output capout;
    PREPARE_OUT(capout);
    PREPARE_IN(capin);
    
    
    xError=ADM_coreXvba::funcs.getCapDecode(&capin,&capout);
    CHECK_ERROR(xError);
    if(Success!=xError)
    {
        ADM_warning("Can't get Xvba decode capabilities\n");
        return false;
    }
    for(int c=0;c<capout.num_of_decodecaps;c++)
    {
        switch(capout.decode_caps_list[c].capability_id)
        {
            case  XVBA_H264:      ADM_info("H264");break;
            case XVBA_VC1:        ADM_info("VC1");break;
            case XVBA_MPEG2_IDCT: ADM_info("MPEG2 IDCT");break;
            case XVBA_MPEG2_VLD : ADM_info("MPEG2 VLD");break;
            default :             ADM_info("???\n");break;
        }
        printf("\n");
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
