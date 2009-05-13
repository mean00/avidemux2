/***************************************************************************
            \file              ADM_ffmpeg_vdpau.cpp  
            \brief Decoder using half ffmpeg/half VDPAU

    The ffmpeg part is to preformat inputs for VDPAU
    VDPAU is loaded dynamically to be able to make a binary
        and have something working even if the target machine
        does not have vdpau


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

extern "C" {
#include "ADM_lavcodec.h"
 #include "ADM_libraries/ADM_ffmpeg/libavcodec/vdpau.h"
}

#include "ADM_default.h"
#ifdef USE_VDPAU
#include "vdpau/vdpau_x11.h"
#include "vdpau/vdpau.h"
#include "ADM_codecs/ADM_codec.h"
#include "ADM_codecs/ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_userInterfaces/ADM_render/GUI_render.h"


static bool vdpauWorking=false;

static ADM_LibWrapper        vdpauDynaLoader;
static VdpDeviceCreateX11    *ADM_createVdpX11;
static VdpDevice             vdpDevice;
static VdpGetProcAddress     *vdpProcAddress;
static VdpDecoder            vdpDecoder;

// VDPAU internal linker

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

static VdpFunctions funcs;

#define WRAP_Open_Template(funcz,argz,display,codecid) \
{\
AVCodec *codec=funcz(argz);\
if(!codec) {GUI_Error_HIG("Codec",QT_TR_NOOP("Internal error finding codec"display));ADM_assert(0);} \
  codecId=codecid; \
  _context->workaround_bugs=1*FF_BUG_AUTODETECT +0*FF_BUG_NO_PADDING; \
  _context->error_concealment=3; \
  if (avcodec_open(_context, codec) < 0)  \
                      { \
                                        printf("[lavc] Decoder init: "display" video decoder failed!\n"); \
                                        GUI_Error_HIG("Codec","Internal error opening "display); \
                                        ADM_assert(0); \
                                } \
                                else \
                                { \
                                        printf("[lavc] Decoder init: "display" video decoder initialized! (%s)\n",codec->long_name); \
                                } \
}

#define WRAP_Open(x)            {WRAP_Open_Template(avcodec_find_decoder,x,#x,x);}
#define WRAP_OpenByName(x,y)    {WRAP_Open_Template(avcodec_find_decoder_by_name,#x,#x,y);}


/**
    \fn vdpauUsable
    \brief Return true if  vdpau can be used...
*/
bool vdpauUsable(void)
{
    return vdpauWorking;
}
/**

*/
static void *getFunc(uint32_t id)
{
    void *f;
    if(VDP_STATUS_OK!=vdpProcAddress(vdpDevice,id,&f)) return NULL;
    return (void *)f;
}
/**
    \fn vdpauProbe
    \brief Try loading vdpau...
*/
bool vdpauProbe(void)
{
    memset(&funcs,0,sizeof(funcs));
    if(false==vdpauDynaLoader.loadLibrary("/usr/lib/libvdpau.so"))
    {
        return false;
    }
    ADM_createVdpX11=(VdpDeviceCreateX11*)vdpauDynaLoader.getSymbol("vdp_device_create_x11");
    if(!ADM_createVdpX11) return false;

    //
    GUI_WindowInfo xinfo;
    void *draw;
    draw=UI_getDrawWidget();
    UI_getWindowInfo(draw,&xinfo );
    
    // try to create....
    if( VDP_STATUS_OK!=ADM_createVdpX11((Display*)xinfo.display,0,&vdpDevice,&vdpProcAddress))
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
        printf("[VDPAU] API : 0x%x, info : %s\n",version,versionString);

    vdpauWorking=true;
    return true;
}
/**

*/
int ADM_getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
    uint32_t w= avctx->width;
    uint32_t h= avctx->height;

    uint32_t rounded_w=(w+63)&~63;
    uint32_t rounded_h=(h+63)&~63;
    uint32_t page=rounded_w*rounded_h;

    pic->data[0]=new uint8_t [page];
    pic->data[1]=new uint8_t [page/4];
    pic->data[2]=new uint8_t [page/4];

    pic->linesize[0]=rounded_w;
    pic->linesize[1]=rounded_w/2;
    pic->linesize[2]=rounded_w/2;
    pic->type= FF_BUFFER_TYPE_USER;
    pic->age=1;
    return 0;
}
/**

*/
 void ADM_releaseBuffer(struct AVCodecContext *avctx, AVFrame *pic)
{
    #define F(x) if(pic->x) delete [] pic->x;pic->x=NULL;
    F(data[0]);
    F(data[1]);
    F(data[2]);
    return;
}
#define NB_SURFACE 12
static VdpVideoSurface surface[NB_SURFACE];
/**
    \fn decoderFFVDPAU
*/
decoderFFVDPAU::decoderFFVDPAU(uint32_t w, uint32_t h, uint32_t l, uint8_t * d):decoderFF (w,	   h)
{
        _context->get_buffer      = ADM_getBuffer;
        _context->release_buffer  = ADM_releaseBuffer;
        _context->reget_buffer    = ADM_getBuffer;

        WRAP_OpenByName(h264_vdpau,CODEC_ID_H264);

        // Now instantiate our VDPAU surface & decoder
        ADM_assert(VDP_STATUS_OK==funcs.decoderCreate(vdpDevice,VDP_DECODER_PROFILE_H264_HIGH,w,h,16,&vdpDecoder));
        // Create our surfaces...
        for(int i=0;i<NB_SURFACE;i++)
        {
            ADM_assert(VDP_STATUS_OK==funcs.createSurface(vdpDevice,VDP_CHROMA_TYPE_420,w,h,&(surface[i])));
        }
        scratch=new ADMImage(w,h,1);
}
/**
    \fn uncompress
*/
uint8_t decoderFFVDPAU::uncompress (ADMCompressedImage * in, ADMImage * out)
{
VdpStatus status;
    static int ping=0;
    // First let ffmpeg prepare datas...
    if(!decoderFF::uncompress (in, scratch))
    {
        printf("[VDPAU] No data from libavcodec\n");
        return 0;
    }
    printf("[VDPAU] OK*0***\n");
    
    // Borrowed from mplayer
    struct vdpau_render_state *rndr = (struct vdpau_render_state *)scratch->_planes[0];
    
    status=funcs.decoderRender(vdpDecoder,surface[ping],
                            (void * const *)&rndr->info, rndr->bitstream_buffers_used, rndr->bitstream_buffers);
    if(VDP_STATUS_OK!=status)
    {
        
        printf("[VDPAU] No data after decoderRender <%s>\n",funcs.getErrorString(status));
        return 0;
    }
    printf("[VDPAU] OK*1***\n");
    
    out->_colorspace=ADM_COLOR_YV12;
    // Now get back our image...
    // Copy back from decompressed stuff to our own
 
    void *planes[3];
            planes[0]=out->GetWritePtr(PLANAR_Y);
            planes[1]=out->GetWritePtr(PLANAR_U);
            planes[2]=out->GetWritePtr(PLANAR_V);
    uint32_t stride[3];
            stride[0]=out->GetPitch(PLANAR_Y);
            stride[1]=out->GetPitch(PLANAR_U);
            stride[2]=out->GetPitch(PLANAR_V);

    status=funcs.getDataSurface(
                surface[ping],
                VDP_YCBCR_FORMAT_YV12, //VdpYCbCrFormat   destination_ycbcr_format,
                planes, //void * const *   destination_data,
                stride //destination_pitches
                );
    if(VDP_STATUS_OK!=status)
    {
        
        printf("[VDPAU] Cannot get data from surface <%s>\n",funcs.getErrorString(status));
        return 0;
    }
    ping++;
    ping%=NB_SURFACE;
    printf("[VDPAU] OK*2***\n");
    out->Pts=in->demuxerPts;
    return 1;
}

#endif
// EOF