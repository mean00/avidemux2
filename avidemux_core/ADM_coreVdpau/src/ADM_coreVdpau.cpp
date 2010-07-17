/***************************************************************************
    \file             : ADM_coreVdpau.cpp
    \brief            : Wrapper around vdpau functions
    \author           : (C) 2010 by mean fixounet@free.fr
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
#include "../include/ADM_coreVdpau.h"
#ifdef USE_VDPAU
#include "ADM_dynamicLoading.h"


GUI_WindowInfo      admVdpau::myWindowInfo;

#define CHECK(x) if(!isOperationnal()) {ADM_error("vdpau is not operationnal\n");return VDP_STATUS_ERROR;}\
                 VdpStatus r=x;\
                 if(VDP_STATUS_OK!=r) {ADM_warning(#x" call failed with error=%s\n",getErrorString(r));}return r;

/**
    \fn VdpFunctions
    
*/
typedef struct 
{
    VdpGetErrorString       *getErrorString;
    VdpGetApiVersion        *getApiVersion;
    VdpGetInformationString *getInformationString;

    VdpVideoSurfaceCreate   *createSurface;
    VdpVideoSurfaceDestroy  *destroySurface;
    VdpVideoSurfaceGetBitsYCbCr *getDataSurface;
    VdpVideoSurfacePutBitsYCbCr *surfacePutBitsYCbCr;

 

    VdpOutputSurfaceCreate  *createOutputSurface;
    VdpOutputSurfaceDestroy *destroyOutputSurface;
    VdpOutputSurfacePutBitsYCbCr *putBitsYV12OutputSurface;
    VdpOutputSurfaceQueryPutBitsYCbCrCapabilities *putBitsCapsOutputSurface;
    VdpOutputSurfaceGetBitsNative                 *getBitsNativeOutputSurface;

    VdpDecoderCreate        *decoderCreate;
    VdpDecoderDestroy       *decoderDestroy;
    VdpDecoderRender        *decoderRender;


    VdpPresentationQueueTargetDestroy *presentationQueueDestroy;
    VdpPresentationQueueCreate        *presentationQueueCreate;
    VdpPresentationQueueGetTime       *presentationQueueGetTime;
    VdpPresentationQueueDisplay       *presentationQueueDisplay;

    VdpVideoMixerCreate               *mixerCreate;
    VdpVideoMixerDestroy              *mixerDestroy;
    VdpVideoMixerRender               *mixerRender;

    VdpPresentationQueueTargetCreateX11 *presentationQueueDisplayX11Create;
}VdpFunctions;

static VdpFunctions          funcs;
static ADM_LibWrapper        vdpauDynaLoader;
static VdpDeviceCreateX11    *ADM_createVdpX11;
static VdpDevice             vdpDevice;
static VdpGetProcAddress     *vdpProcAddress;
static bool                  coreVdpWorking=false;
static VdpPresentationQueueTarget  queueX11;
/**
    \fn getFunc
    \brief vdpau function pointers from ID
*/
static void *getFunc(uint32_t id)
{
    void *f;
    if(VDP_STATUS_OK!=vdpProcAddress(vdpDevice,id,&f)) return NULL;
    return (void *)f;
}

/**
    \fn     init
    \brief
*/
bool admVdpau::init(GUI_WindowInfo *x)
{
    memset(&funcs,0,sizeof(funcs));
    if(false==vdpauDynaLoader.loadLibrary("/usr/lib/libvdpau.so"))
    {
        ADM_info("Cannot load libvdpau.so\n");
        return false;
    }
    ADM_createVdpX11=(VdpDeviceCreateX11*)vdpauDynaLoader.getSymbol("vdp_device_create_x11");
    if(!ADM_createVdpX11) return false;

    //    
    // try to create....
    if( VDP_STATUS_OK!=ADM_createVdpX11((Display*)x->display,0,&vdpDevice,&vdpProcAddress))
    {
        return false;
    }
    // Now that we have the vdpProcAddress, time to get the functions....
#define GetMe(fun,id)         funcs.fun= (typeof(funcs.fun))getFunc(id);ADM_assert(funcs.fun); 
        
    GetMe(getErrorString,VDP_FUNC_ID_GET_ERROR_STRING);
    GetMe(getApiVersion,VDP_FUNC_ID_GET_API_VERSION);
    GetMe(getInformationString,VDP_FUNC_ID_GET_INFORMATION_STRING);

    GetMe(createSurface,VDP_FUNC_ID_VIDEO_SURFACE_CREATE);
    GetMe(destroySurface,VDP_FUNC_ID_VIDEO_SURFACE_DESTROY);
    GetMe(getDataSurface,VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR);
    GetMe(surfacePutBitsYCbCr,VDP_FUNC_ID_VIDEO_SURFACE_PUT_BITS_Y_CB_CR);


    GetMe(decoderCreate,VDP_FUNC_ID_DECODER_CREATE);
    GetMe(decoderDestroy,VDP_FUNC_ID_DECODER_DESTROY);
    GetMe(decoderRender,VDP_FUNC_ID_DECODER_RENDER);
    GetMe(createOutputSurface,VDP_FUNC_ID_OUTPUT_SURFACE_CREATE);
    GetMe(destroyOutputSurface,VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY);
    GetMe(putBitsYV12OutputSurface,VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_Y_CB_CR);
    GetMe(putBitsCapsOutputSurface,VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_Y_CB_CR_CAPABILITIES);
    GetMe(getBitsNativeOutputSurface,VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE);

    GetMe(presentationQueueDestroy,VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY);
    GetMe(presentationQueueCreate,VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE);
    GetMe(presentationQueueGetTime,VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME);
    GetMe(presentationQueueDisplay,VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY);

    GetMe(presentationQueueDisplayX11Create,VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11);

    GetMe(mixerCreate,VDP_FUNC_ID_VIDEO_MIXER_CREATE);
    GetMe(mixerDestroy,VDP_FUNC_ID_VIDEO_MIXER_DESTROY);
    GetMe(mixerRender,VDP_FUNC_ID_VIDEO_MIXER_RENDER);
  
    

    if(VDP_STATUS_OK!=funcs.presentationQueueDisplayX11Create(vdpDevice,x->window,&queueX11))
    {
        ADM_warning("Cannot create X11 Presentation Queue\n");
        return false;
    }

    const char *versionString=NULL;
    uint32_t version=0xff;
        funcs.getInformationString(&versionString);
        funcs.getApiVersion(&version);
        ADM_info("[VDPAU] API : 0x%x, info : %s\n",version,versionString);

    coreVdpWorking=true;
    myWindowInfo=*x;
#if 0
    ADM_info("Checking supported format\n");
// See http://us.download.nvidia.com/XFree86/Linux-x86/195.36.24/README/vdpausupport.html#vdpau-implementation-limits-output-surface
    ADM_info("FORMAT_B8G8R8A8->VDP_YCBCR_FORMAT_NV12 : %d\n",(int)queryYUVPutBitSupport(VDP_RGBA_FORMAT_B8G8R8A8,VDP_YCBCR_FORMAT_NV12));
    ADM_info("FORMAT_B8G8R8A8->VDP_YCBCR_FORMAT_YV12 : %d\n",(int)queryYUVPutBitSupport(VDP_RGBA_FORMAT_B8G8R8A8,VDP_YCBCR_FORMAT_YV12));
    ADM_info("FORMAT_B8G8R8A8->VDP_YCBCR_FORMAT_UYVY : %d\n",(int)queryYUVPutBitSupport(VDP_RGBA_FORMAT_B8G8R8A8,VDP_YCBCR_FORMAT_UYVY));
    ADM_info("FORMAT_B8G8R8A8->VDP_YCBCR_FORMAT_Y8U8V8A8 : %d\n",(int)queryYUVPutBitSupport(VDP_RGBA_FORMAT_B8G8R8A8,VDP_YCBCR_FORMAT_Y8U8V8A8));

    ADM_info("FORMAT_R8G8B8A8->VDP_YCBCR_FORMAT_NV12 : %d\n",(int)queryYUVPutBitSupport(VDP_RGBA_FORMAT_R8G8B8A8,VDP_YCBCR_FORMAT_NV12));
    ADM_info("FORMAT_R8G8B8A8->VDP_YCBCR_FORMAT_YV12 : %d\n",(int)queryYUVPutBitSupport(VDP_RGBA_FORMAT_R8G8B8A8,VDP_YCBCR_FORMAT_YV12));
    ADM_info("FORMAT_R8G8B8A8->VDP_YCBCR_FORMAT_UYVY : %d\n",(int)queryYUVPutBitSupport(VDP_RGBA_FORMAT_R8G8B8A8,VDP_YCBCR_FORMAT_UYVY));
#endif

    ADM_info("VDPAU renderer init ok.\n");
    return true;
}
/**
    \fn queryYUVPutBitSupport
*/
bool admVdpau::queryYUVPutBitSupport(VdpRGBAFormat rgb,VdpYCbCrFormat yuv)
{
    VdpBool c;
    if(VDP_STATUS_OK!=funcs.putBitsCapsOutputSurface(vdpDevice,rgb,yuv,&c))
    {
        ADM_warning("Query YCBCR put bits failed\n");
        return false;
    }
    if(c) return true;
    return false;
}

/**
    \fn isOperationnal
*/
bool admVdpau::isOperationnal(void)
{
    return coreVdpWorking;
}
/**
    \fn
    \brief
*/
VdpStatus admVdpau::decoderCreate( VdpDecoderProfile profile,    uint32_t  width,uint32_t  height,
            uint32_t  max_references,VdpDecoder *      decoder)
{
    CHECK(funcs.decoderCreate(vdpDevice,profile,width,height,max_references,decoder));
}
/**
    \fn
    \brief
*/
VdpStatus  admVdpau::decoderDestroy(VdpDecoder decoder)
{
    CHECK(funcs.decoderDestroy(decoder));
}
/**
    \fn
    \brief
*/

VdpStatus  admVdpau::surfaceCreate(uint32_t width,uint32_t height,VdpVideoSurface *surface)
{
    CHECK(funcs.createSurface(vdpDevice,VDP_CHROMA_TYPE_420,width,height,surface));
}
/**
    \fn
    \brief
*/

VdpStatus  admVdpau::surfaceDestroy(VdpVideoSurface surface)
{
    CHECK(funcs.destroySurface(surface));
}
/**
    \fn
    \brief
*/

VdpStatus  admVdpau::getDataSurface(VdpVideoSurface surface,uint8_t *planes[3],uint32_t stride[3])
{
  CHECK(funcs.getDataSurface(
                surface,
                VDP_YCBCR_FORMAT_YV12, //VdpYCbCrFormat   destination_ycbcr_format,
                ( void * const *)planes, //void * const *   destination_data,
                stride //destination_pitches
                ));
}
/**
    \fn 
    \brief
*/
VdpStatus   admVdpau::surfacePutBits(VdpVideoSurface surface,uint8_t *planes[3],uint32_t stride[3])
{
    CHECK(funcs.surfacePutBitsYCbCr(surface,  
                VDP_YCBCR_FORMAT_YV12, //VdpYCbCrFormat   destination_ycbcr_format,
                ( void * const *)planes, //void * const *   destination_data,
                stride //destination_pitches
                ));
}
/**
    \fn
    \brief
*/

const char *admVdpau::getErrorString(VdpStatus er)
{
    return funcs.getErrorString(er);
}
VdpStatus admVdpau::decoderRender(
    VdpDecoder                 decoder,
    VdpVideoSurface            target,
    const void                 *info,
    uint32_t                   bitstream_buffer_count,
    VdpBitstreamBuffer const * bitstream_buffers)
{
    CHECK(funcs.decoderRender(decoder, target, (void * const *)info,bitstream_buffer_count, bitstream_buffers));
}
/**
    \fn
    \brief
*/

VdpStatus admVdpau::outputSurfaceCreate(
    VdpRGBAFormat      rgba_format,
    uint32_t           width,
    uint32_t           height,
    VdpOutputSurface * surface)
{
    CHECK(funcs.createOutputSurface(vdpDevice,rgba_format, width,height,surface));
}
/**
    \fn
    \brief
*/

VdpStatus admVdpau::outputSurfaceDestroy(    VdpOutputSurface surface)
{
    CHECK(funcs.destroyOutputSurface(surface));
}
/**
    \fn
    \brief
*/

VdpStatus admVdpau::outPutSurfacePutBitsYV12( VdpOutputSurface     surface,
                        uint8_t *planes[3],
                        uint32_t pitches[3])
{
    CHECK(funcs.putBitsYV12OutputSurface(surface,VDP_YCBCR_FORMAT_YV12,
                                                       (void const * const *) planes,
                                                        pitches,
                                                        NULL,//VdpRect const *      destination_rect,
                                                        NULL)); //VdpCSCMatrix const * csc_matrix  );
}
/**
    \fn outputSurfaceGetBitsNative
*/
VdpStatus admVdpau::outputSurfaceGetBitsNative(VdpOutputSurface     surface, uint8_t *buffer, uint32_t w,uint32_t h)
{
    // Only support RGBA 32
    uint32_t pitches[3]={w*4,0,0};
    uint8_t *ptr[4]={buffer,NULL,NULL};
    CHECK(funcs.getBitsNativeOutputSurface( surface,
    NULL, // Rect
     ( void * const *)ptr,
    pitches));
}

/**
    \fn
    \brief
*/

VdpStatus admVdpau::presentationQueueCreate(VdpPresentationQueue *queue)
{
    CHECK(funcs.presentationQueueCreate(vdpDevice,queueX11,queue));

}
/**
    \fn
    \brief
*/

VdpStatus admVdpau::presentationQueueDestroy(VdpPresentationQueue queue)
{
    CHECK(funcs.presentationQueueDestroy(queue));
}
/**
    \fn
    \brief
*/

VdpStatus admVdpau::presentationQueueDisplay(VdpPresentationQueue queue,VdpOutputSurface outputSurface)
{
    VdpTime t;
    VdpStatus z=funcs.presentationQueueGetTime(queue,&t);
    if(VDP_STATUS_OK!=z)
    {
        ADM_warning("GetTime failed\n");
        return z;
    }
    CHECK(funcs.presentationQueueDisplay(queue,outputSurface,0,0,t));
}
/**
    \fn mixerCreate
*/
VdpStatus admVdpau::mixerCreate(uint32_t width,uint32_t height, VdpVideoMixer *mixer)
{
#define MIXER_NB_PARAM 3

VdpVideoMixerParameter parameters[MIXER_NB_PARAM]=
                                              {VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH,
                                               VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT,
                                               VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE};
uint32_t color=VDP_CHROMA_TYPE_420;
void    *values[MIXER_NB_PARAM]={&width,&height,&color};
    
    
    VdpStatus e=funcs.mixerCreate(vdpDevice,
                        0,NULL,
                        MIXER_NB_PARAM,parameters,values,
                        mixer);
    if(VDP_STATUS_OK!=e)
    {
        
        ADM_warning("MixerCreate  failed :%s\n",getErrorString(e));
        
    }
    return e;
}
/**
    \fn mixerDestroy
*/

VdpStatus admVdpau::mixerDestroy(VdpVideoMixer mixer)
{
    CHECK(funcs.mixerDestroy(mixer));
}
/**
    \fn mixerRender
*/

VdpStatus admVdpau::mixerRender(VdpVideoMixer mixer,
                                VdpVideoSurface sourceSurface,
                                VdpOutputSurface targetOutputSurface, 
                                uint32_t targetWidth, 
                                uint32_t targetHeight )
{
const VdpVideoSurface listOfSurface[1]={sourceSurface};
const VdpVideoSurface listOfInvalidSurface[1]={VDP_INVALID_HANDLE};
      VdpStatus e=funcs.mixerRender(mixer,
                VDP_INVALID_HANDLE,NULL,    // Background
                VDP_VIDEO_MIXER_PICTURE_STRUCTURE_FRAME,
                
                0,            listOfInvalidSurface, // Past...
                sourceSurface,                      // current
                0,            listOfInvalidSurface, // Future
                NULL,                               // source RECT
                targetOutputSurface,
                NULL,                               // dest Rec
                NULL,                               // dest video Rec
                0,NULL);                            // Layers
                
            
  if(VDP_STATUS_OK!=e)
    {
        
        ADM_warning("MixerCreate  failed :%s\n",getErrorString(e));
        
    }
    return e;
}

#else 
//******************************************
//******************************************
// Dummy when vdpau is not there...
// Dummy when vdpau is not there...
//******************************************
//******************************************
static bool                  coreVdpWorking=false;
bool admVdpau::init(GUI_WindowInfo *x)
{
          return false;
}
  
/**
    \fn isOperationnal
*/
bool admVdpau::isOperationnal(void)
{
    ADM_warning("This binary has no VPDAU support\n");
    return coreVdpWorking;
}
#endif
// EOF
