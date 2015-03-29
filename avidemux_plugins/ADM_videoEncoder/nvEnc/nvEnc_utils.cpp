
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//
// Some part derived from ffmpeg

#include "ADM_default.h"
#include "nvEncodeAPI.h"
#include "ADM_dynamicLoading.h"

static bool probeDone=false;
static bool nvEncAvailable=false;
/**
 */

#if defined(_WIN32)
    #define CUDA_LIB "nvcuda.dll"
    #define MOD_TYPE HDMODULE cuda_lib;
    #define CUDAAPI __stdcall
    #ifdef ADM_CPU_X86_64
        #define NVENC_LIB "nvEncodeAPI64.dll"
    #else   
        #define NVENC_LIB "nvEncodeAPI.dll"
    #endif
#else
    #define CUDAAPI 
    #define NVENC_LIB   "libnvidia-encode.so.1"
    #define MOD_TYPE    HDMODULE void *;
    #define CUDA_LIB    "libcuda.so"
#endif

typedef enum cudaError_enum {
    CUDA_SUCCESS = 0
} CUresult;
typedef int CUdevice;
typedef void* CUcontext;
/**
 * 
 * @param file
 */
class ADM_nvCudaLoader : public ADM_LibWrapper
{
public:
    ADM_nvCudaLoader()
    {
        this->initialised = loadLibrary(CUDA_LIB) && getSymbols(8,  &init,               "cuInit",
                                                                    &getDeviceCount,      "cuDeviceGetCount",
                                                                    &getDevice,           "cuDeviceGet",
                                                                    &getDeviceName,       "cuDeviceGetName",
                                                                    &getDeviceCapabilities,"cuDeviceComputeCapability",
                                                                    &createContext,       "cuCtxCreate_v2",
                                                                    &popCurrentContext,   "cuCtxPopCurrent_v2",               
                                                                    &contextDestroy,      "cuCtxDestroy_v2"                
                            );
    }
    bool isAvailable()
    {
        return initialised;
    }
public:
  CUresult CUDAAPI (*init)(unsigned int Flags);
  CUresult CUDAAPI (*getDeviceCount)(int *count);
  CUresult CUDAAPI (*getDevice)(CUdevice *device, int ordinal);
  CUresult CUDAAPI (*getDeviceName)(char *name, int len, CUdevice dev);
  CUresult CUDAAPI (*getDeviceCapabilities)(int *major, int *minor, CUdevice dev);
  CUresult CUDAAPI (*createContext)(CUcontext *pctx, unsigned int flags, CUdevice dev);
  CUresult CUDAAPI (*popCurrentContext)(CUcontext *pctx);
  CUresult CUDAAPI (*contextDestroy)(CUcontext ctx);  
};
/**
 * 
 * @param file
 */
class ADM_nvNvEncLoader : public ADM_LibWrapper
{
public:
    ADM_nvNvEncLoader()
    {
        this->initialised = loadLibrary(NVENC_LIB) && getSymbols(1,  &createInstance,      "NvEncodeAPICreateInstance");
        if(initialised)
        {
            functionList.version=NV_ENCODE_API_FUNCTION_LIST_VER;
            if(createInstance(&functionList))
            {
                ADM_warning("libNvEnc: Failed to load functions block");
                initialised=false;
            }
        }
    }
    bool isAvailable()
    {
        return initialised;
    }
public:
  
  NVENCSTATUS CUDAAPI           (*createInstance)(NV_ENCODE_API_FUNCTION_LIST *functionList);  
  NV_ENCODE_API_FUNCTION_LIST   functionList;
  
};
static ADM_nvCudaLoader   *cudaLoader=NULL; 
static ADM_nvNvEncLoader  *nvEncLoader=NULL;
static CUdevice            selectedDevice=-1;
/**
 * 
 */
class nvEncSession
{
public:
                nvEncSession();
                ~nvEncSession();
        bool    init(void);
public:
        bool    createContext(void);
        bool    openSession(void);
protected:
        bool                            inited;
        void                            *encoder;
        NV_ENCODE_API_FUNCTION_LIST     *fonctions;
        CUcontext                       context;
};

/**
 * 
 * @param avctx
 * @return 
 */
bool  loadCuda()
{
        cudaLoader=new ADM_nvCudaLoader();
        // 1- Load cuda
        if(!cudaLoader->isAvailable())
        {
            delete cudaLoader;
            cudaLoader=NULL;
            return false;
        }
        return true;
}
/**
 * 
 * @param er
 * @param c
 * @return 
 */
static bool cudaCheck(int er, const char *c)
{
    if(er)
    {
        ADM_warning("Error %d when calling %s\n",er,c);
        return false;
    }
    return true;
}
//#define cudaCall(x) cudaLoader->x //return cudaCheck(cudaLoader->x,#x);
#define cudaCall(x)         cudaCheck(cudaLoader->x,#x)
#define cudaAbortOnFail(x) if(!cudaCall(x)) goto abortCudaProbe;
/**
 * 
 * @param avctx
 * @return 
 */
static bool  probeCuda()
{
   
    ADM_info( "Probing cuda\n");
    if(!loadCuda())
    {
        ADM_warning("Cannot load cuda\n");
        return false;
    }
    ADM_warning("Cuda loaded, probing..\n");
    if(!cudaCall(init(0)))
        return false;

    int deviceCount=0;
    if(!cudaCall(getDeviceCount(&deviceCount)))
        return false;

    if (!deviceCount) 
    {
        ADM_warning( "No Cuda device available\n");
        return false;
    }

    ADM_info( "found %d CUDA devices \n", deviceCount);

    for (int i = 0; i < deviceCount; ++i) 
    {
        CUdevice dev;
        char chipName[128];
        int major,minor,ver;
        
        cudaAbortOnFail(getDevice(&dev,i));
        cudaAbortOnFail(getDeviceName(chipName,sizeof(chipName),dev));
        cudaAbortOnFail(getDeviceCapabilities(&major,&minor,dev));
        ver = (major << 4) | minor;
        ADM_info("Found  chip, GPU %s, SM %d.d",chipName,major,minor);
        if(ver>=0x30)
        {
            ADM_info("   this chip has nvenc");
            if(!nvEncAvailable)
            {
                nvEncAvailable=true;
                selectedDevice=dev;
            }
        }
    }

    return nvEncAvailable;
abortCudaProbe:
    return false;
            
}
/**
 * 
 * @return 
 */
bool loadNvEnc()
{
   if(probeDone) return nvEncAvailable;
    probeDone=true;
    nvEncAvailable=false;
    if(!probeCuda()) return false;    
    
     nvEncLoader=new ADM_nvNvEncLoader;
     nvEncAvailable=nvEncLoader->isAvailable();
     if(!nvEncAvailable)
     {
         delete nvEncLoader;
         nvEncLoader=NULL;
     }
     if(nvEncAvailable) ADM_info("NvEnc lib loaded OK\n");
     else ADM_warning("NvEnc lib failed to load\n");
     
     nvEncSession s;
     s.init();
     s.createContext();
     s.openSession();
             
     
     return nvEncAvailable;
}


#define CHECK_ERROR(a,b) {int z=a;if(z!=NV_ENC_SUCCESS) {ADM_warning("Error : %d\n",z);b;}}
#define INIT_CHECK() {if(!inited) {ADM_warning("nvEncSession not initialized\n");return false;}}
/*
 * 
 */
nvEncSession::nvEncSession()
{
    inited=false;  
}
/**
 * 
 */
nvEncSession::~nvEncSession()
{
    fonctions=NULL;
    inited=false;  
}
/**
 * 
 * @return 
 */
bool    nvEncSession::init(void)
{
    if(!nvEncAvailable)
    {
        return false;
    }
    fonctions=&(nvEncLoader->functionList);
    inited=true;
    return true;
}
/**
 * 
 * @return 
 */
bool nvEncSession::createContext(void)
{
     INIT_CHECK();
    
     
     if(!cudaCall(createContext(&context,0,selectedDevice)))
     {
         context=NULL;
         return false;
     }
     return true;
}

/**
 * 
 */
bool    nvEncSession::openSession(void)    
{
    INIT_CHECK();
    ADM_assert(fonctions->nvEncOpenEncodeSessionEx);
    if(!context)
    {
        ADM_warning("No context\n");
        return false;
    }
    NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS params;
        params.version=   NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
        params.deviceType=NV_ENC_DEVICE_TYPE_CUDA; 
        params.device=context;
        params.apiVersion=NVENCAPI_VERSION;

    CHECK_ERROR(fonctions->nvEncOpenEncodeSessionEx(&params, &encoder),return false;);
    return true;
}
//--

// EOF



