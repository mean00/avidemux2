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
#include "ADM_dynamicLoading.h"

#ifdef USE_VDPAU
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

    VdpDecoderCreate        *decoderCreate;
    VdpDecoderDestroy       *decoderDestroy;
    VdpDecoderRender        *decoderRender;
}VdpFunctions;

static VdpFunctions          funcs;
static ADM_LibWrapper        vdpauDynaLoader;
static VdpDeviceCreateX11    *ADM_createVdpX11;
static VdpDevice             vdpDevice;
static VdpGetProcAddress     *vdpProcAddress;
static bool                  coreVdpWorking=false;
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

    GetMe(decoderCreate,VDP_FUNC_ID_DECODER_CREATE);
    GetMe(decoderDestroy,VDP_FUNC_ID_DECODER_DESTROY);
    GetMe(decoderRender,VDP_FUNC_ID_DECODER_RENDER);



    const char *versionString=NULL;
    uint32_t version=0xff;
        funcs.getInformationString(&versionString);
        funcs.getApiVersion(&version);
        ADM_info("[VDPAU] API : 0x%x, info : %s\n",version,versionString);

    coreVdpWorking=true;
    return true;
}
/**
    \fn isOperationnal
*/
bool admVdpau::isOperationnal(void)
{
    return coreVdpWorking;
}

VdpStatus admVdpau::decoderCreate( VdpDecoderProfile profile,    uint32_t  width,uint32_t  height,
            uint32_t  max_references,VdpDecoder *      decoder)
{
    return funcs.decoderCreate(vdpDevice,profile,width,height,max_references,decoder);
}
VdpStatus  admVdpau::decoderDestroy(VdpDecoder decoder)
{
    return funcs.decoderDestroy(decoder);
}
VdpStatus  admVdpau::surfaceCreate(uint32_t width,uint32_t height,VdpVideoSurface *surface)
{
return funcs.createSurface(vdpDevice,VDP_CHROMA_TYPE_420,width,height,surface);
}
VdpStatus  admVdpau::surfaceDestroy(VdpVideoSurface surface)
{
    return funcs.destroySurface(surface);
}
VdpStatus  admVdpau::getDataSurface(VdpVideoSurface surface,uint8_t *planes[3],uint32_t stride[3])
{
  return funcs.getDataSurface(
                surface,
                VDP_YCBCR_FORMAT_YV12, //VdpYCbCrFormat   destination_ycbcr_format,
                ( void * const *)planes, //void * const *   destination_data,
                stride //destination_pitches
                );
}
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
    return funcs.decoderRender(decoder, target, (void * const *)info,bitstream_buffer_count, bitstream_buffers);
}
#else // Dummy when vdpau is not there...

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

VdpStatus admVdpau::decoderCreate( VdpDecoderProfile profile,    uint32_t  width,uint32_t  height,
            uint32_t  max_references,VdpDecoder *      decoder)
{
    ADM_assert(0);
}
VdpStatus  admVdpau::decoderDestroy(VdpDecoder decoder)
{
    ADM_assert(0);
}
VdpStatus  admVdpau::surfaceCreate(uint32_t width,uint32_t height,VdpVideoSurface *surface)
{
ADM_assert(0);
}
VdpStatus  admVdpau::surfaceDestroy(VdpVideoSurface surface)
{
    ADM_assert(0);
}
VdpStatus  admVdpau::getDataSurface(VdpVideoSurface surface,uint8_t *planes[3],uint32_t stride[3])
{
  ADM_assert(0);
}
const char *admVdpau::getErrorString(VdpStatus er)
{
ADM_assert(0);
}
VdpStatus admVdpau::decoderRender(
    VdpDecoder                 decoder,
    VdpVideoSurface            target,
    const void                 *info,
    uint32_t                   bitstream_buffer_count,
    VdpBitstreamBuffer const * bitstream_buffers)
{
    ADM_assert(0);
}

#endif
// EOF
