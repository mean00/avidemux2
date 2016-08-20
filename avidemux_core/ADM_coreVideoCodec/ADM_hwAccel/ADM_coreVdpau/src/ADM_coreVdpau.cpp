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
#include "../include/ADM_coreVdpauInternal.h"
#include "ADM_dynamicLoading.h"
#include <map>

#if 0
#define VDP_TRACK printf
#else
#define VDP_TRACK(...) {}
#endif

GUI_WindowInfo      admVdpau::myWindowInfo;


typedef std::map<VdpVideoSurface ,bool >::iterator myIterator;

namespace ADM_coreVdpau
{
 VdpFunctions          funcs;
 VdpDevice             vdpDevice;
}

static ADM_LibWrapper        vdpauDynaLoader;
static VdpDeviceCreateX11    *ADM_createVdpX11;
static VdpGetProcAddress     *vdpProcAddress;
static bool                  coreVdpWorking=false;
static VdpPresentationQueueTarget  queueX11;



static std::map <VdpVideoSurface ,bool >listOfAllocatedSurface;
/**
 * \fn admVdpau_exitCleanup
 */
bool admVdpau_exitCleanup()
{
    std::map <VdpVideoSurface ,bool > cpy=listOfAllocatedSurface;
    int n=cpy.size();
    printf("At exit, we have still %d surface\n",n);
    myIterator it=cpy.begin();
    for(;it!=cpy.end();it++)
        admVdpau::surfaceDestroy(it->first);
    printf("After cleanup we have  %d surface\n",(int)listOfAllocatedSurface.size());
    
    admVdpau::cleanup();
    return true;
}


/**
    \fn getFunc
    \brief vdpau function pointers from ID
*/
static void *getFunc(uint32_t id)
{
    void *f;
    if(VDP_STATUS_OK!=vdpProcAddress(ADM_coreVdpau::vdpDevice,id,&f)) return NULL;
    return (void *)f;
}
/**

*/
void        *admVdpau::getVdpDevice(void)
{
        return (void *)(intptr_t)ADM_coreVdpau::vdpDevice;
}
/**

*/
void        *admVdpau::getProcAddress(void)
{
    printf("==> GetProcAddress called\n");
    return (void *)vdpProcAddress;
}
VdpGetProcAddress        *admVdpau::getProcAddress2(void)
{
    printf("==> GetProcAddress called\n");
    return vdpProcAddress;
}

/**
    \fn     init
    \brief
*/
bool admVdpau::init(GUI_WindowInfo *x)
{
    memset(&ADM_coreVdpau::funcs,0,sizeof(ADM_coreVdpau::funcs));
    if(false==vdpauDynaLoader.loadLibrary("libvdpau.so"))
    {
        ADM_info("Cannot load libvdpau.so\n");
        return false;
    }
    ADM_createVdpX11=(VdpDeviceCreateX11*)vdpauDynaLoader.getSymbol("vdp_device_create_x11");
    if(!ADM_createVdpX11) return false;

    //    
    // try to create....
    if( VDP_STATUS_OK!=ADM_createVdpX11((Display*)x->display,0,&ADM_coreVdpau::vdpDevice,&vdpProcAddress))
    {
        return false;
    }
    // Now that we have the vdpProcAddress, time to get the functions....
#if 0 //def HAS_MOVE_SEMANTICS
#define myTypeOf   decltype
#else
#define myTypeOf   typeof
#endif
#define GetMe(fun,id)         ADM_coreVdpau::funcs.fun= (myTypeOf(ADM_coreVdpau::funcs.fun))getFunc(id);ADM_assert(ADM_coreVdpau::funcs.fun); 
        
    GetMe(deviceDestroy,VDP_FUNC_ID_DEVICE_DESTROY);
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
    GetMe(mixerEnableFeatures,VDP_FUNC_ID_VIDEO_MIXER_SET_FEATURE_ENABLES);
    GetMe(mixerQueryFeatureSupported,VDP_FUNC_ID_VIDEO_MIXER_QUERY_FEATURE_SUPPORT);
    GetMe(mixerGetFeaturesEnabled,VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_ENABLES);

    GetMe(mixerGetAttributesValue,VDP_FUNC_ID_VIDEO_MIXER_GET_ATTRIBUTE_VALUES);
    GetMe(mixerSetAttributesValue,VDP_FUNC_ID_VIDEO_MIXER_SET_ATTRIBUTE_VALUES);
    GetMe(mixerGetOutputSurfaceParameters,VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS); ///
    GetMe(mixerGetSurfaceParameters,VDP_FUNC_ID_VIDEO_SURFACE_GET_PARAMETERS); ///
    if(VDP_STATUS_OK!=ADM_coreVdpau::funcs.presentationQueueDisplayX11Create(ADM_coreVdpau::vdpDevice,x->systemWindowId,&queueX11))
    {
        ADM_warning("Cannot create X11 Presentation Queue\n");
        return false;
    }

    const char *versionString=NULL;
    uint32_t version=0xff;
        ADM_coreVdpau::funcs.getInformationString(&versionString);
        ADM_coreVdpau::funcs.getApiVersion(&version);
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
    ADM_info("Vdpau supports VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL : %d\n",(int)mixerFeatureSupported(VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL));
    ADM_info("Vdpau supports VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL         : %d\n",(int)mixerFeatureSupported(VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL));
    ADM_info("Vdpau supports VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L5      : %d\n",(int)mixerFeatureSupported(VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L5));
    ADM_info("Vdpau supports VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L3      : %d\n",(int)mixerFeatureSupported(VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L3));
    ADM_info("Vdpau supports VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1      : %d\n",(int)mixerFeatureSupported(VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1));
    ADM_info("Vdpau supports VDP_VIDEO_MIXER_FEATURE_NOISE_REDUCTION              : %d\n",(int)mixerFeatureSupported(VDP_VIDEO_MIXER_FEATURE_NOISE_REDUCTION));
    ADM_info("Vdpau supports VDP_VIDEO_MIXER_FEATURE_INVERSE_TELECINE             : %d\n",(int)mixerFeatureSupported(VDP_VIDEO_MIXER_FEATURE_INVERSE_TELECINE));

    ADM_info("VDPAU renderer init ok.\n");
    return true;
}
/**
    \fn cleanup
*/
bool admVdpau::cleanup(void)
{
    if(true==coreVdpWorking)
    {
            ADM_info("Destroying vdp device..\n");
            ADM_coreVdpau::funcs.deviceDestroy(ADM_coreVdpau::vdpDevice);
            ADM_coreVdpau::vdpDevice=VDP_INVALID_HANDLE;
    }
    coreVdpWorking=false;
    return true;
}
/**
    \fn queryYUVPutBitSupport
*/
bool admVdpau::queryYUVPutBitSupport(VdpRGBAFormat rgb,VdpYCbCrFormat yuv)
{
    VdpBool c;
    if(VDP_STATUS_OK!=ADM_coreVdpau::funcs.putBitsCapsOutputSurface(ADM_coreVdpau::vdpDevice,rgb,yuv,&c))
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
VdpStatus admVdpau::decoderCreate(  VdpDevice dev,VdpDecoderProfile profile,    uint32_t  width,uint32_t  height,
            uint32_t  max_references,VdpDecoder *      decoder)
{
    VDP_TRACK("Creating decoder\n");
    CHECK(ADM_coreVdpau::funcs.decoderCreate(dev,profile,width,height,max_references,decoder));
}
/**
    \fn
    \brief
*/
VdpStatus  admVdpau::decoderDestroy(VdpDecoder decoder)
{
    VDP_TRACK("Destroying decoder\n");
    CHECK(ADM_coreVdpau::funcs.decoderDestroy(decoder));
}
/**
    \fn
    \brief
*/

VdpStatus  admVdpau::surfaceCreate(uint32_t width,uint32_t height,VdpVideoSurface *surface)
{
    
    if(!isOperationnal()) 
        {ADM_error("vdpau is not operationnal\n");return VDP_STATUS_ERROR;}
    VdpStatus r=ADM_coreVdpau::funcs.createSurface(ADM_coreVdpau::vdpDevice,VDP_CHROMA_TYPE_420,width,height,surface);
    if(VDP_STATUS_OK!=r) 
    {
        ADM_warning("ADM_coreVdpau::funcs.createSurface(ADM_coreVdpau::vdpDevice,VDP_CHROMA_TYPE_420,width,height,surface) call failed with error=%s\n",getErrorString(r));
       return r;
    }
    myIterator already = listOfAllocatedSurface.find(*surface);
    if(already!=listOfAllocatedSurface.end())
    {
        ADM_assert("Doubly used vdpau surface\n");
    }
    listOfAllocatedSurface[*surface]=true;
    return VDP_STATUS_OK;
}
/**
    \fn
    \brief
*/

VdpStatus  admVdpau::surfaceDestroy(VdpVideoSurface surface)
{
    // De we have it ?
     myIterator already = listOfAllocatedSurface.find(surface);
    if(already==listOfAllocatedSurface.end())
    {
        ADM_assert("Trying to destroy unallocated vdpau surface\n");
    }
    listOfAllocatedSurface.erase(already);
    CHECK(ADM_coreVdpau::funcs.destroySurface(surface));
}
/**
    \fn
    \brief
*/

VdpStatus  admVdpau::getDataSurface(VdpVideoSurface surface,uint8_t *planes[3],uint32_t stride[3])
{
  CHECK(ADM_coreVdpau::funcs.getDataSurface(
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
    CHECK(ADM_coreVdpau::funcs.surfacePutBitsYCbCr(surface,  
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
    return ADM_coreVdpau::funcs.getErrorString(er);
}
VdpStatus admVdpau::decoderRender(
    VdpDecoder                 decoder,
    VdpVideoSurface            target,
    const void                 *info,
    uint32_t                   bitstream_buffer_count,
    VdpBitstreamBuffer const * bitstream_buffers)
{
    VDP_TRACK("Calling VDPAU decoder , target surface=%d\n",target);
    CHECK(ADM_coreVdpau::funcs.decoderRender(decoder, target, (void * const *)info,bitstream_buffer_count, bitstream_buffers));
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
    CHECK(ADM_coreVdpau::funcs.createOutputSurface(ADM_coreVdpau::vdpDevice,rgba_format, width,height,surface));
}
/**
    \fn
    \brief
*/

VdpStatus admVdpau::outputSurfaceDestroy(    VdpOutputSurface surface)
{
    CHECK(ADM_coreVdpau::funcs.destroyOutputSurface(surface));
}
/**
    \fn
    \brief
*/

VdpStatus admVdpau::outPutSurfacePutBitsYV12( VdpOutputSurface     surface,
                        uint8_t *planes[3],
                        uint32_t pitches[3])
{
    CHECK(ADM_coreVdpau::funcs.putBitsYV12OutputSurface(surface,VDP_YCBCR_FORMAT_YV12,
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
    CHECK(ADM_coreVdpau::funcs.getBitsNativeOutputSurface( surface,
    NULL, // Rect
     ( void * const *)ptr,
    pitches));
}
/**
    \fn outputSurfaceGetBitsNative_FieldWeave
*/
VdpStatus admVdpau::outputSurfaceGetBitsNative_FieldWeave(VdpOutputSurface     surface, uint8_t *buffer, uint32_t w,uint32_t h)
{
    // Only support RGBA 32
    uint32_t pitches[3]={w*8,0,0};
    uint8_t *ptr[4]={buffer,NULL,NULL,NULL};
    CHECK(ADM_coreVdpau::funcs.getBitsNativeOutputSurface( surface,
    NULL, // Rect
     ( void * const *)ptr,
    pitches));
}


/**
    \fn
    \brief
*/
VdpStatus admVdpau::outputSurfaceGetParameters(  VdpOutputSurface surface,    VdpRGBAFormat *  rgba_format,
                            uint32_t *       width,    uint32_t *       height)
{
    CHECK(ADM_coreVdpau::funcs.mixerGetOutputSurfaceParameters(surface,rgba_format,width,height));
}
/**
 */
VdpStatus admVdpau::surfaceGetParameters(VdpVideoSurface surface,VdpChromaType *chroma,uint32_t *w,uint32_t *h)
{
    CHECK(ADM_coreVdpau::funcs.mixerGetSurfaceParameters(surface,chroma,w,h));
}

/**
    \fn
    \brief
*/

VdpStatus admVdpau::presentationQueueCreate(VdpPresentationQueue *queue)
{
    CHECK(ADM_coreVdpau::funcs.presentationQueueCreate(ADM_coreVdpau::vdpDevice,queueX11,queue));

}
/**
    \fn
    \brief
*/

VdpStatus admVdpau::presentationQueueDestroy(VdpPresentationQueue queue)
{
    CHECK(ADM_coreVdpau::funcs.presentationQueueDestroy(queue));
}
/**
    \fn    presentationQueueDisplay
    \brief display immediately the outputsurface
*/

VdpStatus admVdpau::presentationQueueDisplay(VdpPresentationQueue queue,VdpOutputSurface outputSurface)
{
    VdpTime t;
    VdpStatus z=ADM_coreVdpau::funcs.presentationQueueGetTime(queue,&t);
    if(VDP_STATUS_OK!=z)
    {
        ADM_warning("GetTime failed\n");
        return z;
    }
    CHECK(ADM_coreVdpau::funcs.presentationQueueDisplay(queue,outputSurface,0,0,t));
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
bool admVdpau::cleanup(void)
{
    ADM_warning("This binary has no VPDAU support\n");
    return true;
}
#endif
// EOF
