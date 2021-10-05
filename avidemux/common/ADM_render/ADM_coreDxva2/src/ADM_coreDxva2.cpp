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
#include "ADM_coreD3DApi.h" // Must be the 1st included
#include "ADM_default.h"
#include "ADM_coreD3D.h"
#ifdef USE_DXVA2

#define DUMP_GUID
#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif



#include "ADM_dynamicLoading.h"
#include <map>

#include "../include/ADM_coreDxva2.h"

typedef IDirect3D9* WINAPI pDirect3DCreate9(UINT);
typedef HRESULT     WINAPI pCreateDeviceManager9(UINT *, IDirect3DDeviceManager9 **);

static bool                  coreDxva2Working=false;

static ADM_LibWrapper        dxva2DynaLoader;

static pCreateDeviceManager9 *createDeviceManager = NULL;

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
DEFINE_GUID(DXVA2_ModeHEVC_VLD_Main10,0x107af0e0, 0xef1a,0x4d19,0xab,0xa8,0x67,0xa1,0x63,0x07,0x3d,0x13);
DEFINE_GUID(GUID_NULL,                0x00000000, 0x0000,0x0000,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
/**
 */
typedef struct dxva2_mode
{
  const GUID     *guid;
  enum AVCodecID codec;
  int            bitsPerChannel;
} dxva2_mode;
/**
 */
static int ALIGN(int x,int align,bool verbose=false)
{
    int y= ((x+(align-1)) &(~(align-1)));
    if(verbose)
        printf("Align %d,%d => %d\n",x,align,y);
    return y;
}

/**
 */
static const dxva2_mode dxva2_modes[] =
{
    /* MPEG-2 */
    { &DXVA2_ModeMPEG2_VLD,      AV_CODEC_ID_MPEG2VIDEO ,8},
    { &DXVA2_ModeMPEG2and1_VLD,  AV_CODEC_ID_MPEG2VIDEO ,8},

    /* H.264 */
    { &DXVA2_ModeH264_F,         AV_CODEC_ID_H264 ,8},
    { &DXVA2_ModeH264_E,         AV_CODEC_ID_H264 ,8},
    /* Intel specific H.264 mode */
    { &DXVADDI_Intel_ModeH264_E, AV_CODEC_ID_H264 ,8},

    /* VC-1 / WMV3 */
    { &DXVA2_ModeVC1_D2010,      AV_CODEC_ID_VC1  ,8},
    { &DXVA2_ModeVC1_D2010,      AV_CODEC_ID_WMV3 ,8},
    { &DXVA2_ModeVC1_D,          AV_CODEC_ID_VC1  ,8},
    { &DXVA2_ModeVC1_D,          AV_CODEC_ID_WMV3 ,8},

    /* HEVC/H.265 */
    { &DXVA2_ModeHEVC_VLD_Main,   AV_CODEC_ID_HEVC ,8},
    { &DXVA2_ModeHEVC_VLD_Main10, AV_CODEC_ID_HEVC ,10},

    /* VP8/9 */
    { &DXVA2_ModeVP9_VLD_Profile0,AV_CODEC_ID_VP9 ,8},

    { NULL,                      (AVCodecID)0 ,0},
};
typedef struct
{
     enum AVCodecID  codec;
     bool            enabled;
     unsigned int    verifiedWorkingWidth;
     unsigned int    verifiedWorkingHeight;
     unsigned int    verifiedNotWorkingWidth;
     unsigned int    verifiedNotWorkingHeight;
     DXVA2_VideoDesc desc;
     DXVA2_ConfigPictureDecode pictureDecode;
           GUID      device_guid;
           GUID      guid;
}Dxv2SupportMap;

static Dxv2SupportMap dxva2H265={AV_CODEC_ID_HEVC,false,0,0,INT_MAX,INT_MAX};
static Dxv2SupportMap dxva2H264={AV_CODEC_ID_H264,false,0,0,INT_MAX,INT_MAX};
static Dxv2SupportMap dxva2H265_10Bits={AV_CODEC_ID_HEVC,false,0,0,INT_MAX,INT_MAX};

/**
  \fn dxvaBitsToFormat
  \brief returns D3D format depending on bits per component
*/
static D3DFORMAT dxvaBitsToFormat(int bits)
{
  switch(bits)
  {
    case 8: return (D3DFORMAT)MKTAG('N','V','1','2');break;
    case 10:return (D3DFORMAT)MKTAG('P','0','1','0');break;
    default:
        ADM_assert(0); break;
  }
  return (D3DFORMAT)MKTAG('N','V','1','2');
}
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
  \fn checkDecoder
  \brief check if the decoder advertised is actually working
*/
static bool checkDecoderConfiguration(const GUID *guid, Dxv2SupportMap *context, unsigned int width, unsigned int height)
{
    HRESULT hr;
    unsigned int cfg_count = 0, best_score = 0;
    DXVA2_ConfigPictureDecode *cfg_list = NULL;
    bool   found=false;
    const D3DFORMAT target_format = (const D3DFORMAT) MKTAG('N','V','1','2');

    // initialize desc with values to test
    context->desc.SampleWidth=width;
    context->desc.SampleHeight=height;
    context->desc.Format=target_format;

    hr = D3DCall(IDirectXVideoDecoderService,GetDecoderConfigurations,decoder_service, *guid, &(context->desc), NULL, &cfg_count, &cfg_list);
    if (ADM_FAILED(hr)) {
        ADM_warning("Unable to retrieve decoder configurations\n");
        context->verifiedNotWorkingWidth = width;
        context->verifiedNotWorkingHeight = height;
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
            context->verifiedWorkingWidth = width;
            context->verifiedWorkingHeight = height;
            context->pictureDecode = *cfg;
            found=true;
            ADM_info("\t best score\n");
        }
    }
    CoTaskMemFree(cfg_list);

    if (! found )
    {
        ADM_warning( "No valid decoder configuration available\n");
        context->verifiedNotWorkingWidth = width;
        context->verifiedNotWorkingHeight = height;
        return false;
    }
    return true;
}
/**
 * \fn lookupCodec
 * \brief find and populate the configuration for a given codec
 */
static bool lookupCodec(const char *codecName,
                        Dxv2SupportMap *context,
                        unsigned int guid_count,
                        GUID *guid_list,
                        int bitsPerComponent,
                        unsigned int width,
                        unsigned int height)
{
    bool success=false;
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
#ifdef DUMP_GUID
    dumpGUID("Potential device_guid",*(mode->guid));
#endif

        if (j == guid_count)
            continue;

        hr = D3DCall(IDirectXVideoDecoderService,GetDecoderRenderTargets,decoder_service,*( mode->guid), &target_count, &target_list);
        if (ADM_FAILED(hr))
        {
            ADM_info("Cannot GetDecoderRenderTargets\n");
            continue;
        }
        D3DFORMAT tgt;
        tgt=dxvaBitsToFormat(bitsPerComponent);
        for (j = 0; j < target_count; j++)
        {
            const D3DFORMAT format = target_list[j];
            if (format ==tgt)
            {
                target_format = format;
                break;
            }
        }

        CoTaskMemFree(target_list);
        if (target_format)
        {
            if(checkDecoderConfiguration(mode->guid,context,width,height)) // does it work for given width and height?
            {
              success=true;
              context->device_guid = *(mode->guid);
              break;
            }
        }
    }

    // Did we find a working one ?
    if (IsEqualGUID(context->device_guid, GUID_NULL) || !success)
    {
        ADM_info("No decoder device for codec %s %d bits and size %u x %u found\n",codecName,bitsPerComponent,width,height);
        return false;
    }

    //--

    ADM_info("DXVA2 support for %s %d bits and size %u x %u found\n",codecName,bitsPerComponent,width,height);
    context->enabled=true;
    return true;
}
/**
 * \fn init
 * \brief initialize the low level common part of dxva2
 */
bool admDxva2::init(GUI_WindowInfo *x)
{
   if(!admD3D::isOperationnal())
   {
     ADM_warning("No D3D, skipping DXVA2\n");
     return false;
   }
   if(!admD3D::getDevice())
   {
     ADM_warning("No D3D device, skipping DXVA2\n");
     return false;
   }
    UINT adapter = D3DADAPTER_DEFAULT;
    D3DDISPLAYMODE        d3ddm;
    D3DPRESENT_PARAMETERS d3dpp = {0};

    HRESULT hr;
    unsigned int resetToken = 0;

    // Load dxva2 dll
    if(false==dxva2DynaLoader.loadLibrary("dxva2.dll"))
    {
        ADM_warning("Dxva2: Cannot load dxva2.dll\n");
        goto failInit;
    }

    createDeviceManager=(pCreateDeviceManager9 *)dxva2DynaLoader.getSymbol("DXVA2CreateDirect3DDeviceManager9");
    if(!createDeviceManager)
    {
        ADM_warning("Dxva2: Cannot get symbol  createDeviceManager\n");
        goto failInit;
    }
    hr = createDeviceManager(&resetToken, &d3d9devmgr);
    if (ADM_FAILED(hr))
    {
        ADM_warning( "Failed to create Direct3D device manager\n");
        goto failInit;
    }
    hr = D3DCall(IDirect3DDeviceManager9,ResetDevice,d3d9devmgr, admD3D::getDevice(), resetToken);
    if (ADM_FAILED(hr))
    {
        ADM_warning( "Failed to bind Direct3D device to device manager\n");
        goto failInit;
    }
    ADM_info("Dxva2 init step1\n");
    hr = D3DCall(IDirect3DDeviceManager9,OpenDeviceHandle,d3d9devmgr, &deviceHandle);
    if (ADM_FAILED(hr)) {
         ADM_warning("Failed to open device handle\n");
        goto failInit;
    }

    hr = D3DCall(IDirect3DDeviceManager9,GetVideoService,d3d9devmgr, deviceHandle, (REFIID )IID_IDirectXVideoDecoderService, (void **)&decoder_service);
    if (ADM_FAILED(hr)) {
        ADM_warning("Failed to create IDirectXVideoDecoderService\n");
        goto failInit;
    }

    coreDxva2Working=true;
    ADM_info("Dxva2 core successfully initialized\n");
    ADM_info("Decoder service=%p\n",decoder_service);
    // Look up what is supported
    {
        unsigned int guid_count = 0;
        GUID *guid_list = NULL;

        hr = D3DCall(IDirectXVideoDecoderService,GetDecoderDeviceGuids,decoder_service, &guid_count, &guid_list);
        if (ADM_FAILED(hr)) {
            ADM_warning("Failed to retrieve decoder device GUIDs\n");
            goto failInit;
        }
        const unsigned int dummyCodedW=1280;
        const unsigned int dummyCodedH=720;
        lookupCodec("H264",&dxva2H264,guid_count,guid_list,8,dummyCodedW,dummyCodedH);
        lookupCodec("H265",&dxva2H265,guid_count,guid_list,8,dummyCodedW,dummyCodedH);
        lookupCodec("H265",&dxva2H265_10Bits,guid_count,guid_list,10,dummyCodedW,dummyCodedH);
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
admDx2Surface         *admDx2Surface::duplicateForMe(IDirect3DDevice9 *me)
{
#if 0
    HRESULT hr;
    LPDIRECT3DSURFACE9 surfaces=NULL;
    HANDLE sh=NULL;
    sh=this->sharedHandle;
    if(!sh)
        return NULL;
    hr = D3DCall(IDirect3DDevice9Ex,CreateOffscreenPlainSurface,
                                                   (IDirect3DDevice9Ex *)me,
                                                   this->width,
                                                   this->height,
                                                   this->format, 
                                                   D3DPOOL_DEFAULT, 
                                                   &surfaces, &sh);
    if(ADM_FAILED(hr))
    {
        ADM_warning("Cannot duplicate surface\n");
        return NULL;
     }
    admDx2Surface *s=new admDx2Surface(this->parent,this->alignment);
    s->width=this->width;
    s->height=this->height;
    s->parent=NULL;
    s->sharedHandle=NULL;
    s->surface=surfaces;
    return s;
#endif
return NULL;
}

/**
 */
bool admDxva2::allocateDecoderSurface(void *parent,int w, int h,int align,int num, LPDIRECT3DSURFACE9 *surfaces, std::vector<admDx2Surface *>&listOf,int bits)
{
    HRESULT hr;
    HANDLE sh=NULL;
    int width=ALIGN(w,align);
    int height=ALIGN(h,align);
    D3DFORMAT fmt;
    fmt=dxvaBitsToFormat(bits);

    bool share=admD3D::isDirect9Ex();
    if(share)
    {
     hr = D3DCall(IDirectXVideoDecoderService,CreateSurface,decoder_service,
                                                   width,
                                                   height,
                                                   num-1,
                                                   fmt, D3DPOOL_DEFAULT, 0,
                                                   DXVA2_VideoDecoderRenderTarget,
                                                   surfaces, &sh);
        if(ADM_FAILED(hr))
        {
            // Requesting resource sharing on Windows 7 may result in a failure to allocate surfaces.
            // On Windows 10, not requesting a shared handle seems to impact energy efficiency.
            ADM_warning("Cannot allocate D3D9 surfaces with resource sharing, retrying without.\n");
            share=false;
        }
    }
    if(!share)
    {
     hr = D3DCall(IDirectXVideoDecoderService,CreateSurface,decoder_service,
                                                   width,
                                                   height,
                                                   num-1,
                                                   fmt, D3DPOOL_DEFAULT, 0,
                                                   DXVA2_VideoDecoderRenderTarget,
                                                   surfaces, NULL);
    }
     if(ADM_FAILED(hr))
     {
         ADM_warning("Cannot allocate D3D9 surfaces\n");
         return false;
     }
     ADM_info("Allocated surface %p with shared handle=%p\n",surfaces,sh);
     for(int i=0;i<num;i++)
     {
         admDx2Surface *s=new admDx2Surface(parent,align);
         s->surface=surfaces[i];
         s->width=width;
         s->height=height;
         s->format=fmt;
         s->sharedHandle=sh;
         listOf.push_back(s);
     }
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
                D3DCallNoArg(IDirect3DSurface9,Release,surfaces[i]);
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
    if(coreDxva2Working) return true;
    ADM_warning("This binary has no working DXVA2 support\n");
    return false;
}
/**
 */
bool admDxva2::cleanup(void)
{
    if (decoder_service)
    {
        D3DCallNoArg(IDirectXVideoDecoderService,Release,decoder_service);
        decoder_service=NULL;
    }
    if (d3d9devmgr && deviceHandle != INVALID_HANDLE_VALUE)
    {
        D3DCall(IDirect3DDeviceManager9,CloseDeviceHandle,d3d9devmgr, deviceHandle);
        deviceHandle=INVALID_HANDLE_VALUE;
    }

    if (d3d9devmgr)
    {
        D3DCallNoArg(IDirect3DDeviceManager9,Release,d3d9devmgr);
        d3d9devmgr  =NULL;
    }

    if (d3d9device)
    {
        D3DCallNoArg(IDirect3DDevice9,Release,d3d9device);
        d3d9device  =NULL;
    }


    coreDxva2Working=false;

    // small memleak  d3dDynaLoader;
    // small memleak dxva2DynaLoader;
    return true;
}
/**
 *
 */
bool admDxva2::supported(AVCodecID codec, int bits, int width, int height)
{
    if(!coreDxva2Working || (bits != 8 && bits != 10))
        return false;

#define SUPSUP(a,b,c) \
    if(codec==a && bits==c) \
    { \
        if(!b.enabled) return false; \
        if((int)b.verifiedWorkingWidth >= width && (int)b.verifiedWorkingHeight >= height) return true; \
        if((int)b.verifiedNotWorkingWidth <= width && (int)b.verifiedNotWorkingHeight <= height) return false; \
    }
    SUPSUP(AV_CODEC_ID_H264,dxva2H264,8)
    SUPSUP(AV_CODEC_ID_HEVC,dxva2H265,8)
    SUPSUP(AV_CODEC_ID_HEVC,dxva2H265_10Bits,10)

    unsigned int guid_count = 0;
    GUID *guid_list = NULL;

    HRESULT hr = D3DCall(IDirectXVideoDecoderService, GetDecoderDeviceGuids, decoder_service, &guid_count, &guid_list);
    if(ADM_FAILED(hr))
    {
        ADM_warning("Failed to retrieve decoder device GUIDs\n");
        return false;
    }
    bool r = false;
    if(codec == AV_CODEC_ID_H264 && bits == 8)
        r = lookupCodec("H264",&dxva2H264,guid_count,guid_list,8,width,height);
    else if(codec == AV_CODEC_ID_HEVC && bits == 8)
        r = lookupCodec("H265",&dxva2H265,guid_count,guid_list,8,width,height);
    else if(codec == AV_CODEC_ID_HEVC && bits == 10)
        r = lookupCodec("H265",&dxva2H265_10Bits,guid_count,guid_list,10,width,height);
    CoTaskMemFree(guid_list);
    return r;
}
/**
 * \fn getDecoderConfig
 */
DXVA2_ConfigPictureDecode *admDxva2::getDecoderConfig(AVCodecID codec,int bits)
{
    Dxv2SupportMap *cmap;
    switch(codec)
    {
        case AV_CODEC_ID_H264: cmap=&dxva2H264;break;
        case AV_CODEC_ID_H265:
              if(10==bits)
                cmap=&dxva2H265_10Bits;
              else
                cmap=&dxva2H265;
              break;
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
IDirectXVideoDecoder  *admDxva2::createDecoder(AVCodecID codec, int with, int height, int numSurface, LPDIRECT3DSURFACE9 *surface,int align,int bits)
{
    Dxv2SupportMap *cmap;
    int paddedWidth=ALIGN(with,align,true);
    int paddedHeight=ALIGN(height,align,true);
    switch(codec)
    {
        case AV_CODEC_ID_H264:
                ADM_info("Creating decoder DXVA2/H264/8 Bits\n");
                cmap=&dxva2H264;
                break;
        case AV_CODEC_ID_H265:
                if(bits==10)
                {
                    ADM_info("Creating decoder DXVA2/H264/10 Bits\n");
                    cmap=&dxva2H265_10Bits;
                }
                else
                {
                    ADM_info("Creating decoder DXVA2/H264/8 Bits\n");
                    cmap=&dxva2H265;
                }
                break;
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
    hr = D3DCall(IDirectXVideoDecoderService,CreateVideoDecoder,decoder_service,
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
 * \fn createDecoder
 */
bool admDxva2::destroyDecoder(IDirectXVideoDecoder *decoder)
{
    D3DCallNoArg(IDirectXVideoDecoder,Release,decoder);
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
admDx2Surface::admDx2Surface(void *par,int alig)
{
    parent=par;
    alignment=alig;
    surface=NULL;
    decoder=NULL;
    refCount=0;
    color10Bits=NULL;
    width=height=0;
    format=D3DFMT_UNKNOWN;
    sharedHandle=NULL;
}
/**
 * \fn dtor
 */
admDx2Surface::~admDx2Surface()
{
    parent=NULL;
    surface=NULL;
    decoder=NULL;
    if(color10Bits)
    {
        delete color10Bits;
        color10Bits=NULL;
    }
}
/**
 * \fn addRef
 */
bool admDx2Surface::addRef()
{
    D3DCallNoArg(IDirect3DSurface9,AddRef,surface);
    return true;
}
/**
 * \fn removeRef
 */
bool admDx2Surface::removeRef()
{
     D3DCallNoArg(IDirect3DSurface9,Release,surface); // ???
     return true;
}


/**
 * \fn ADM_DXVA2_readBack
 */
bool  admDx2Surface::surfaceToAdmImage( ADMImage *out)
{
    D3DSURFACE_DESC    surfaceDesc;
    D3DLOCKED_RECT     LockedRect;
    HRESULT            hr;
    int bits=8;
    bool r=true;
    D3DCall(IDirect3DSurface9,GetDesc,surface, &surfaceDesc);
    if(surfaceDesc.Format==(D3DFORMAT)MKTAG('P','0','1','0'))
    {
      bits=10;
    }
    aprintf("Surface to admImage = %p\n",surface);
    hr = D3DCall(IDirect3DSurface9,LockRect,surface, &LockedRect, NULL, D3DLOCK_READONLY);
    if (ADM_FAILED(hr))
    {
        ADM_warning("Unable to lock DXVA2 surface\n");
        return false;
    }
    aprintf("Retrieving image pitch=%d width=%d height=%d\n",LockedRect.Pitch,out->GetWidth(PLANAR_Y), out->GetHeight(PLANAR_Y));

    uint8_t *data=(uint8_t*)LockedRect.pBits;
    int sourcePitch=LockedRect.Pitch;
    switch(bits)
    {
      case 8:   out->convertFromNV12(data,data+sourcePitch*ALIGN(out->GetHeight(PLANAR_Y),alignment), sourcePitch, sourcePitch);
                break;
      case 10:
              {
              if(!color10Bits)
                    color10Bits=new ADMColorScalerSimple(out->GetWidth(PLANAR_Y),out->GetHeight(PLANAR_Y),ADM_PIXFRMT_NV12_10BITS,ADM_PIXFRMT_YV12);
              ADMImageRef ref(out->GetWidth(PLANAR_Y),out->GetHeight(PLANAR_Y));
              ADMImageRefWrittable target(out->GetWidth(PLANAR_Y),out->GetHeight(PLANAR_Y));
              ref._planes[0]=data;
              ref._planeStride[0]=sourcePitch;
              ref._planes[1]=data+((surfaceDesc.Height)*sourcePitch);
              ref._planeStride[1]=sourcePitch;
              ref._planes[2]=NULL;
              ref._planeStride[2]=0;
              // U &  V are inverted...
              for(int i=0;i<3;i++)
              {
                  int j=i;
                  //if(j) j=j^3; // U & V swap is handled by convertImage
                  target._planes[j]=out->GetWritePtr((ADM_PLANE)i);
                  target._planeStride[j]=out->_planeStride[i];
              }
              color10Bits->convertImage(&ref,&target);
              }
              break;

      default: ADM_warning("Unsupported bit depth");break;
    }
    D3DCallNoArg(IDirect3DSurface9,UnlockRect,surface);
    return r;
}

/**
 */
bool admDxva2::decoderAddRef(IDirectXVideoDecoder *decoder)
{
    HRESULT            hr=D3DCallNoArg(IDirectXVideoDecoder,AddRef,decoder);
    return true;
}
/**
 */
bool admDxva2::decoderRemoveRef(IDirectXVideoDecoder *decoder)
{
    HRESULT            hr=D3DCallNoArg(IDirectXVideoDecoder,Release,decoder);
    return true;
}


#endif
// EOF
