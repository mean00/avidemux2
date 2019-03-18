#pragma once

#include "ADM_dynamicLoading.h"

#if !defined(__APPLE__) && !defined(_WIN32)
 #include <dlfcn.h>
#endif




#ifdef __APPLE__
        #define  DLL_TO_LOAD "libvapoursynth-script.dylib"
        #define  PYTHONLIB ""
#else
    #ifdef _WIN32
        #define  DLL_TO_LOAD "vsscript.dll"
        #define  PYTHONLIB ""
    #else
        #define  DLL_TO_LOAD "libvapoursynth-script.so"
        #define  PYTHONLIB VAPOURSYNTH_PYTHONLIB
    #endif
#endif


/**
 * 
 */
class vsDynaLoader:public ADM_LibWrapper
{
public:
  
    vsDynaLoader()
    {
      init=NULL;
      getVSApi=NULL;
      freeScript=NULL;
      finalize=NULL;
      getError=NULL;
      getOutput=NULL;
      evaluateFile=NULL;
      operational=false;
    }
    bool vsInit(const char *dllName,const char *pythonLib)
    {
#if !defined(__APPLE__) && !defined(_WIN32)      
      ADM_info("Trying to dlopen %s\n",VAPOURSYNTH_PYTHONLIB);
      dlopen(VAPOURSYNTH_PYTHONLIB, RTLD_LAZY|RTLD_GLOBAL);
#endif      
      bool loaded= loadLibrary(dllName);
      if(!loaded) 
      {
          ADM_warning("Cannot load the vapoursynth-script library\n");
          return false;
      }
      if(!     ADM_LibWrapper::getSymbols(7,
				&init,          "vsscript_init",
                                &getVSApi,      "vsscript_getVSApi",
                                &freeScript,    "vsscript_freeScript",
                                &finalize,      "vsscript_finalize",
                                &getError,      "vsscript_getError",
                                &getOutput,     "vsscript_getOutput",
                                &evaluateFile,  "vsscript_evaluateFile"))
        {
            ADM_warning("Cannot get symbols from vapoursynthlibrary\n");
            return false;
        }
      operational=true;
      return true;
    }
    bool isOperational()
    {
      return operational;
    }
public:  
    int             (*init)(void);
    const VSAPI     *(*getVSApi)(void);
    void            (*freeScript)(VSScript *handle);
    int             (*finalize)(void);
    const char      *(*getError)(VSScript *handle);
    VSNodeRef *      (*getOutput)(VSScript *handle, int index);
    int              (*evaluateFile)(VSScript **handle, const char *scriptFilename, int flags);
protected:    
    bool             operational;
};