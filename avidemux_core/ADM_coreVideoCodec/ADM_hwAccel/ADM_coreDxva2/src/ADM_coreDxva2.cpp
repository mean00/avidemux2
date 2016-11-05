/***************************************************************************
    \file             : ADM_coreDXVA2.cpp
    \brief            : Wrapper around DXVA2 functions
    \author           : (C) 2016 by mean fixounet@free.fr

        This code is strongly derived from ffmpeg_dxva2 from ffmpeg

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

#ifdef USE_DXVA2

#if 1
#define aprintf printf
#define DUMP_GUID
#else
#define aprintf(...) {}
#endif



#include "ADM_dynamicLoading.h"
#include <map>

#define CINTERFACE
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600
#define DXVA2API_USE_BITFIELDS
#define COBJMACROS
#include <d3d9.h>
#include <dxva2api.h>
#include "../include/ADM_coreDxva2Internal.h"
#include "../include/ADM_coreDxva2.h"

typedef IDirect3D9* WINAPI pDirect3DCreate9(UINT);
typedef HRESULT     WINAPI pCreateDeviceManager9(UINT *, IDirect3DDeviceManager9 **);

static bool                  coreDxva2Working=false;

static ADM_LibWrapper        d3dDynaLoader;
static ADM_LibWrapper        dxva2DynaLoader;

static pDirect3DCreate9      *createD3D = NULL;
static pCreateDeviceManager9 *createDeviceManager = NULL;

static IDirect3D9                  *d3d9        =NULL;
static IDirect3DDevice9            *d3d9device  =NULL;
static IDirect3DDeviceManager9     *d3d9devmgr  =NULL;
static IDirectXVideoDecoderService *decoder_service=NULL;
static HANDLE                       deviceHandle=INVALID_HANDLE_VALUE;
    
#include <initguid.h>
DEFINE_GUID(IID_IDirectXVideoDecoderService, 0xfc51a551,0xd5e7,0x11d9,0xaf,0x55,0x00,0x05,0x4e,0x43,0xff,0x02);

DEFINE_GUID(DXVA2_ModeMPEG2_VLD,      0xee27417f, 0x5e28,0x4e65,0xbe,0xea,0x1d,0x26,0xb5,0x08,0xad,0xc9);
DEFINE_GUID(DXVA2_ModeMPEG2and1_VLD,  0x86695f12, 0x340e,0x4f04,0x9f,0xd3,0x92,0x53,0xdd,0x32,0x74,0x60);
DEFINE_GUID(DXVA2_ModeH264_E,         0x1b81be68, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);
DEFINE_GUID(DXVA2_ModeH264_F,         0x1b81be69, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);
DEFINE_GUID(DXVADDI_Intel_ModeH264_E, 0x604F8E68, 0x4951,0x4C54,0x88,0xFE,0xAB,0xD2,0x5C,0x15,0xB3,0xD6);
DEFINE_GUID(DXVA2_ModeVC1_D,          0x1b81beA3, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);
DEFINE_GUID(DXVA2_ModeVC1_D2010,      0x1b81beA4, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);
DEFINE_GUID(DXVA2_ModeHEVC_VLD_Main,  0x5b11d51b, 0x2f4c,0x4452,0xbc,0xc3,0x09,0xf2,0xa1,0x16,0x0c,0xc0);
DEFINE_GUID(DXVA2_ModeVP9_VLD_Profile0, 0x463707f8, 0xa1d0,0x4585,0x87,0x6d,0x83,0xaa,0x6d,0x60,0xb8,0x9e);
DEFINE_GUID(DXVA2_NoEncrypt,          0x1b81beD0, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);
DEFINE_GUID(GUID_NULL,                0x00000000, 0x0000,0x0000,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
/**
 */
typedef struct dxva2_mode 
{
  const GUID     *guid;
  enum AVCodecID codec;
} dxva2_mode;
/**
 */
static int ALIGN(int x,int alig) 
{
    int y= ((x+(align-1)) &(~(align-1)));
    aprintf("Align %d,%d => %d\n",x,align,y);
    return y;
}

/**
 */
static const dxva2_mode dxva2_modes[] = 
{
    /* MPEG-2 */
    { &DXVA2_ModeMPEG2_VLD,      AV_CODEC_ID_MPEG2VIDEO },
    { &DXVA2_ModeMPEG2and1_VLD,  AV_CODEC_ID_MPEG2VIDEO },

    /* H.264 */
    { &DXVA2_ModeH264_F,         AV_CODEC_ID_H264 },
    { &DXVA2_ModeH264_E,         AV_CODEC_ID_H264 },
    /* Intel specific H.264 mode */
    { &DXVADDI_Intel_ModeH264_E, AV_CODEC_ID_H264 },

    /* VC-1 / WMV3 */
    { &DXVA2_ModeVC1_D2010,      AV_CODEC_ID_VC1  },
    { &DXVA2_ModeVC1_D2010,      AV_CODEC_ID_WMV3 },
    { &DXVA2_ModeVC1_D,          AV_CODEC_ID_VC1  },
    { &DXVA2_ModeVC1_D,          AV_CODEC_ID_WMV3 },

    /* HEVC/H.265 */
    { &DXVA2_ModeHEVC_VLD_Main,  AV_CODEC_ID_HEVC },

    /* VP8/9 */
    { &DXVA2_ModeVP9_VLD_Profile0, AV_CODEC_ID_VP9 },

    { NULL,                      (AVCodecID)0 },
};
typedef struct 
{
     enum AVCodecID  codec;
     bool            enabled;
     DXVA2_VideoDesc desc;
     DXVA2_ConfigPictureDecode pictureDecode;
           GUID      device_guid;
           GUID      guid;
}Dxv2SupportMap;

static Dxv2SupportMap dxva2H265={AV_CODEC_ID_HEVC,false};
static Dxv2SupportMap dxva2H264={AV_CODEC_ID_H264,false};
/**
 */
static void dumpGUID(const char *name,const GUID &guid)
{
    printf("%s :\n",name);
    printf("0x%lx 0x%lx 0x%lx ",guid.Data1,guid.Data2,guid.Data3);
    for(int i=0;i<8;i++) printf(" %02x ",guid.Data4[i]);
    printf("\n");
}

/**
 */
static bool ADM_FAILED(HRESULT hr)
{
    int r=(int)hr;
    if(r<0)
    {
        ADM_warning("Failed with error code=0x%x\n",r);
        return true;
    }
    return false;
}

/**
 * \fn lookupCodec
 */
static bool lookupCodec(const char *codecName,Dxv2SupportMap *context,unsigned int guid_count,GUID *guid_list)
{
    HRESULT hr;
    D3DFORMAT target_format = (D3DFORMAT) 0;

    for (int i = 0; dxva2_modes[i].guid; i++) 
    {
        D3DFORMAT *target_list = NULL;
        unsigned target_count = 0;
        const dxva2_mode *mode = &dxva2_modes[i];
        if (mode->codec != context->codec)
            continue;
        int j;
        for (j = 0; j < guid_count; j++) 
        {
            if (IsEqualGUID( *(mode->guid), guid_list[j]))
                break;
        }
#ifdef(DUMP_GUID)
    dumpGUID("Potential device_guid",*(mode->guid));

#endif    

        if (j == guid_count)
            continue;

        hr = IDirectXVideoDecoderService_GetDecoderRenderTargets(decoder_service,*( mode->guid), &target_count, &target_list);
        if (ADM_FAILED(hr)) 
        {
            continue;
        }
        for (j = 0; j < target_count; j++) 
        {
            const D3DFORMAT format = target_list[j];
            if (format == MKTAG('N','V','1','2')) 
            {
                target_format = format;
                break;
            }
        }
        
        CoTaskMemFree(target_list);
        if (target_format) 
        {
            context->device_guid = *(mode->guid);
            break;
        }
    }
    

    if (IsEqualGUID(context->device_guid, GUID_NULL))
    {
        ADM_info("No decoder device for codec %s found\n",codecName);
        return false;
    }
    //--
    unsigned int cfg_count = 0, best_score = 0;
    DXVA2_ConfigPictureDecode *cfg_list = NULL;
    bool   found=false;
    // initialize desc with dummy values
    context->desc.SampleWidth=1920;
    context->desc.SampleHeight=1080;
    context->desc.Format=target_format;
        
    hr = IDirectXVideoDecoderService_GetDecoderConfigurations(decoder_service, context->device_guid, &(context->desc), NULL, &cfg_count, &cfg_list);
    if (ADM_FAILED(hr)) {
        ADM_warning("Unable to retrieve decoder configurations\n");
        return false;
    }

    for (int i = 0; i < cfg_count; i++) 
    {
        DXVA2_ConfigPictureDecode *cfg = &cfg_list[i];

        unsigned score;
        if (cfg->ConfigBitstreamRaw == 1)
        {
            ADM_info("\t has raw bistream\n");
            score = 1;
        }
        else if (context->codec == AV_CODEC_ID_H264 && cfg->ConfigBitstreamRaw == 2)
        {
            ADM_info("\t has h264 raw bistream2\n");
            score = 2;
        }
        else
            continue;
        if (IsEqualGUID(cfg->guidConfigBitstreamEncryption, DXVA2_NoEncrypt))
        {
            ADM_info("\t has no encrypt\n");
            score += 16;
        }
        if (score > best_score) 
        {
            best_score    = score;
            context->pictureDecode = *cfg;
            found=true;
            ADM_info("\t best score\n");
        }
    }
    CoTaskMemFree(cfg_list);

    if (! found ) 
    {
        ADM_warning( "No valid decoder configuration available\n");
        return false;
    }
    //--
    
    ADM_info("DXVA2 support for  %s found\n",codecName);
    context->enabled=true;
    return true;
}
/**
 * \fn init
 * \brief initialize the low level common part of dxva2
 */
bool admDxva2::init(GUI_WindowInfo *x)
{
    UINT adapter = D3DADAPTER_DEFAULT;   
    D3DDISPLAYMODE        d3ddm;
    D3DPRESENT_PARAMETERS d3dpp = {0};

    HRESULT hr;
    unsigned int resetToken = 0;
    
    // Load d3d9 dll
    if(false==d3dDynaLoader.loadLibrary("d3d9.dll"))
    {
        ADM_warning("Dxva2: Cannot load d3d9.dll\n"); // memleak
        goto failInit;
    }
    // Load d3d9 dll
    if(false==dxva2DynaLoader.loadLibrary("dxva2.dll"))
    {
        ADM_warning("Dxva2: Cannot load dxva2.dll\n");
        goto failInit;
    }
    
    createD3D=(pDirect3DCreate9 *)d3dDynaLoader.getSymbol("Direct3DCreate9");
    if(!createD3D)
    {
        ADM_warning("Dxva2: Cannot get symbol  Direct3DCreate9\n");
        goto failInit;
    }
    createDeviceManager=(pCreateDeviceManager9 *)dxva2DynaLoader.getSymbol("DXVA2CreateDirect3DDeviceManager9");
    if(!createDeviceManager)
    {
        ADM_warning("Dxva2: Cannot get symbol  createDeviceManager\n");
        goto failInit;
    }     
    d3d9 = createD3D(D3D_SDK_VERSION);
    if(!d3d9)
    {
        ADM_warning("Dxva2: Cannot gcreate d3d9\n");
        goto failInit;
    }
    IDirect3D9_GetAdapterDisplayMode(d3d9, adapter, &d3ddm);
    d3dpp.Windowed         = TRUE;
    d3dpp.BackBufferWidth  = 640;
    d3dpp.BackBufferHeight = 480;
    d3dpp.BackBufferCount  = 0;
    d3dpp.BackBufferFormat = d3ddm.Format;
    d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    d3dpp.Flags            = D3DPRESENTFLAG_VIDEO;

    hr = IDirect3D9_CreateDevice(d3d9, adapter, D3DDEVTYPE_HAL, GetDesktopWindow(),
                                 D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
                                 &d3dpp, &d3d9device);
    if(ADM_FAILED(hr))
    {
        ADM_warning("Cannot create d3d9 device\n");
        goto failInit;
    }
    hr = createDeviceManager(&resetToken, &d3d9devmgr);
    if (ADM_FAILED(hr)) 
    {
        ADM_warning( "Failed to create Direct3D device manager\n");
        goto failInit;
    }
    hr = IDirect3DDeviceManager9_ResetDevice(d3d9devmgr, d3d9device, resetToken);
    if (ADM_FAILED(hr))
    {
        ADM_warning( "Failed to bind Direct3D device to device manager\n");
        goto failInit;
    }
    ADM_info("Dxva2 init step1\n");
    hr = IDirect3DDeviceManager9_OpenDeviceHandle(d3d9devmgr, &deviceHandle);
    if (ADM_FAILED(hr)) {
         ADM_warning("Failed to open device handle\n");
        goto failInit;
    }

    hr = IDirect3DDeviceManager9_GetVideoService(d3d9devmgr, deviceHandle, (REFIID )IID_IDirectXVideoDecoderService, (void **)&decoder_service);
    if (ADM_FAILED(hr)) {
        ADM_warning("Failed to create IDirectXVideoDecoderService\n");
        goto failInit;
    }

    coreDxva2Working=true;
    ADM_info("Dxva2 core successfully initialized\n");
    ADM_info("Decoder service=%p\n",decoder_service);
    // Look up what is supported
    {
        unsigned guid_count = 0, i, j;
        GUID device_guid = GUID_NULL;
        GUID *guid_list = NULL;
        HRESULT hr;
        int surface_alignment;

        hr = IDirectXVideoDecoderService_GetDecoderDeviceGuids(decoder_service, &guid_count, &guid_list);
        if (ADM_FAILED(hr)) {
            ADM_warning("Failed to retrieve decoder device GUIDs\n");
            return true;
        }
        lookupCodec("H264",&dxva2H264,guid_count,guid_list);
        lookupCodec("H265",&dxva2H265,guid_count,guid_list);
        CoTaskMemFree(guid_list);
    }
    ADM_info("Scanning supported format done\n");
    return true;
failInit:
    // cleanup
    cleanup();
    return false;
}
/**
 */
bool admDxva2::allocateD3D9Surface(int num,int w, int h,void *array,int surface_alignment)
{
    HRESULT hr;
    LPDIRECT3DSURFACE9 *surfaces=(LPDIRECT3DSURFACE9 *)array;
    int width=ALIGN(w,surface_alignment);
    int height=ALIGN(h,surface_alignment);
    
     hr = IDirectXVideoDecoderService_CreateSurface(decoder_service,
                                                   width,
                                                   height,
                                                   num - 1,
                                                   (D3DFORMAT)MKTAG('N','V','1','2'), D3DPOOL_DEFAULT, 0,
                                                   DXVA2_VideoDecoderRenderTarget,
                                                   surfaces, NULL);
     if(ADM_FAILED(hr))
     {
         ADM_warning("Cannot allocate D3D9 surfaces");
         return false;
     }
     ADM_info("%d surfaces allocated \n",num);
     return true;
}
/**
 */
bool admDxva2::destroyD3DSurface(int num, void *zsurfaces)
{
    LPDIRECT3DSURFACE9 *surfaces=(LPDIRECT3DSURFACE9 *)zsurfaces;
    for(int i=0;i<num;i++)
    {
         if (surfaces[i])
         {
                IDirect3DSurface9_Release(surfaces[i]);
                surfaces[i]=NULL;
         }
    }
    return true;
}
  
/**
    \fn isOperationnal
*/
bool admDxva2::isOperationnal(void)
{
    ADM_warning("This binary has no working DXVA2 support\n");
    return coreDxva2Working;
}
/**
 */
bool admDxva2::cleanup(void)
{
    ADM_warning("This binary has no DXVA2 support\n");

    if (decoder_service)
    {
        IDirectXVideoDecoderService_Release(decoder_service);    
        decoder_service=NULL;
    }
    if (d3d9devmgr && deviceHandle != INVALID_HANDLE_VALUE)
    {
        IDirect3DDeviceManager9_CloseDeviceHandle(d3d9devmgr, deviceHandle);
        deviceHandle=INVALID_HANDLE_VALUE;
    }

    if (d3d9devmgr)
    {
        IDirect3DDeviceManager9_Release(d3d9devmgr);
        d3d9devmgr  =NULL;
    }

    if (d3d9device)
    {
        IDirect3DDevice9_Release(d3d9device);    
        d3d9device  =NULL;
    }

    if (d3d9)
    {
        IDirect3D9_Release(d3d9);
        d3d9        =NULL;
    }
    coreDxva2Working=false;
    
    // small memleak  d3dDynaLoader;
    // small memleak dxva2DynaLoader;    
    return true;
}
/**
 * 
 */
bool        admDxva2::supported(AVCodecID codec)
{
#define SUPSUP(a,b) if(codec==a) return (b.enabled) ;
    SUPSUP(AV_CODEC_ID_H264,dxva2H264)
    SUPSUP(AV_CODEC_ID_HEVC,dxva2H265)
    return false;
}
/**
 * \fn getDecoderConfig
 */
DXVA2_ConfigPictureDecode *admDxva2::getDecoderConfig(AVCodecID codec)
{
    Dxv2SupportMap *cmap;
    switch(codec)
    {
        case AV_CODEC_ID_H264: cmap=&dxva2H264;break;
        case AV_CODEC_ID_H265: cmap=&dxva2H265;break;
        default:
            ADM_assert(0);
            break;
    }
    if(!cmap->enabled)
    {
        ADM_warning("This decoder is not enabled\n");
        ADM_assert(0);
    }
    return &(cmap->pictureDecode);
}

/**
 * \fn createDecoder
 */
IDirectXVideoDecoder  *admDxva2::createDecoder(AVCodecID codec, int with, int height, int numSurface, LPDIRECT3DSURFACE9 *surface,int align)
{
    Dxv2SupportMap *cmap;
    int paddedWidth=ALIGN(with,align);
    int paddedHeight=ALIGN(height,align);
    switch(codec)
    {
        case AV_CODEC_ID_H264: cmap=&dxva2H264;break;
        case AV_CODEC_ID_H265: cmap=&dxva2H265;break;
        default:
            ADM_assert(0);
            break;
    }
    if(!cmap->enabled)
    {
        ADM_warning("This decoder is not enabled\n");
        ADM_assert(0);
    }
    HRESULT hr;
    IDirectXVideoDecoder *decoder=NULL;
#ifdef DUMP_GUID
    dumpGUID("device_guid",cmap->device_guid);
    dumpGUID("       guid",cmap->guid);
    printf("Decoder service = %p\n",decoder_service);

#endif   
    // update with real values
    cmap->desc.SampleWidth=paddedWidth; // does not work with multiple video ?
    cmap->desc.SampleHeight=paddedHeight;
    //
    hr = IDirectXVideoDecoderService_CreateVideoDecoder(decoder_service,
                                                         (cmap->device_guid),
                                                         &(cmap->desc),
                                                         &(cmap->pictureDecode),
                                                         surface,
                                                         numSurface, 
                                                         &decoder);
     if(ADM_FAILED(hr))
     {
         ADM_warning("Cannot create decoder\n");
         return NULL;
     }
     return decoder;     
}

/**
 * \fn ADM_DXVA2_readBack
 */
bool  admDxva2::surfaceToAdmImage(LPDIRECT3DSURFACE9 surface, ADMImage *out)
{
    D3DSURFACE_DESC    surfaceDesc;
    D3DLOCKED_RECT     LockedRect;
    HRESULT            hr;
    bool r=true;
    IDirect3DSurface9_GetDesc(surface, &surfaceDesc);
    aprintf("Surface to admImage = %p\n",surface);
    
    hr = IDirect3DSurface9_LockRect(surface, &LockedRect, NULL, D3DLOCK_READONLY);
    if (ADM_FAILED(hr)) 
    {
        ADM_warning("Unable to lock DXVA2 surface\n");
        return false;
    }
    printf("Retrieving image pitch=%d width=%d height=%d\n",LockedRect.Pitch,out->GetWidth(PLANAR_Y), out->GetHeight(PLANAR_Y));
    // only copy luma for the moment
    uint8_t *data=(uint8_t*)LockedRect.pBits;
    out->convertFromNV12(data,data+LockedRect.Pitch*out->GetHeight(PLANAR_Y), LockedRect.Pitch, LockedRect.Pitch);
    
    //r=BitBlit(YPLANE(out),out->GetPitch(PLANAR_Y),(uint8_t*)LockedRect.pBits,LockedRect.Pitch,out->GetWidth(PLANAR_Y), out->GetHeight(PLANAR_Y));
    IDirect3DSurface9_UnlockRect(surface);
    return r;
}

/**
 * \fn createDecoder
 */
bool admDxva2::destroyDecoder(IDirectXVideoDecoder *decoder)
{
    IDirectXVideoDecoder_Release(decoder);
    return true;
}

/**
 * \fn admLibVa_exitCleanup
 */
bool admDxva2_exitCleanup()
{
    ADM_info("Dxva2 cleanup begin\n");
    admDxva2::cleanup();
    ADM_info("Dxva2 cleanup end\n");
    return true;
}
/**
 * \fn ctor
 */
admDx2Surface::admDx2Surface(void *par)
{
    parent=par;
    surface=NULL;
    decoder=NULL;
}
/**
 * \fn dtor
 */
admDx2Surface::~admDx2Surface()
{
    parent=NULL;
    surface=NULL;
    decoder=NULL;
}
/**
 * \fn addRef
 */
bool admDx2Surface::addRef()
{
    IDirect3DSurface9_AddRef(surface);   
    return true;
}
/**
 * \fn removeRef
 */
bool admDx2Surface::removeRef()
{
     IDirect3DSurface9_Release(surface); // ???
     return true;
}
/**
 */
bool admDxva2::decoderAddRef(IDirectXVideoDecoder *decoder)
{
    HRESULT            hr=IDirectXVideoDecoder_AddRef(decoder);
    return true;
}
/**
 */
bool admDxva2::decoderRemoveRef(IDirectXVideoDecoder *decoder)
{
    HRESULT            hr=IDirectXVideoDecoder_Release(decoder);
    return true;
}


#endif
// EOF
