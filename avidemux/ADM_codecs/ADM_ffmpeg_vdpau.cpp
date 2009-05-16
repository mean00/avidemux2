/***************************************************************************
            \file              ADM_ffmpeg_vdpau.cpp  
            \brief Decoder using half ffmpeg/half VDPAU

    The ffmpeg part is to preformat inputs for VDPAU
    VDPAU is loaded dynamically to be able to make a binary
        and have something working even if the target machine
        does not have vdpau
    Some part, especially get/buffer and ip_age borrowed from xbmc
        as the api from ffmpeg is far from clear....


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
}
#include "ADM_default.h"
#ifdef USE_VDPAU
extern "C" {
 #include "ADM_libraries/ADM_ffmpeg/libavcodec/vdpau.h"
}

#include "vdpau/vdpau_x11.h"
#include "vdpau/vdpau.h"
#include "ADM_codecs/ADM_codec.h"
#include "ADM_codecs/ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_userInterfaces/ADM_render/GUI_render.h"
#include "ADM_ffmpeg_vdpau_internal.h"
#include "prefs.h"

static VdpFunctions funcs;

static bool vdpauWorking=false;

static ADM_LibWrapper        vdpauDynaLoader;
static VdpDeviceCreateX11    *ADM_createVdpX11;
static VdpDevice             vdpDevice;
static VdpGetProcAddress     *vdpProcAddress;

#define aprintf(...) {}

/**
    \fn vdpauUsable
    \brief Return true if  vdpau can be used...
*/
bool vdpauUsable(void)
{
    uint32_t v=false;
    if(!vdpauWorking) return false;
    if(!prefs->get(FEATURE_VDPAU,&v)) v=false;
    return v;
}
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
    \fn ADM_VDPAUgetBuffer
    \brief trampoline to get a VDPAU surface
*/
int ADM_VDPAUgetBuffer(AVCodecContext *avctx, AVFrame *pic)
{
    decoderFFVDPAU *dec=(decoderFFVDPAU *)avctx->opaque;
    return dec->getBuffer(avctx,pic);
}
/**
    \fn getBuffer
    \brief returns a VDPAU render masquerading as a AVFrame
*/
int decoderFFVDPAU::getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
    vdpau_render_state * render;
    if(VDPAU->freeQueue.size()==0)
    {
        printf("[VDPAU] No more available surface\n");
        return -1;
    }
    // Get an image   
    render=VDPAU->freeQueue.back();
    VDPAU->freeQueue.pop_back();
    render->state=0;
    pic->data[0]=(uint8_t *)render;
    pic->data[1]=(uint8_t *)render;
    pic->data[2]=(uint8_t *)render;
    pic->linesize[0]=0;
    pic->linesize[1]=0;
    pic->linesize[2]=0;
    pic->type=FF_BUFFER_TYPE_USER;
    render->state |= FF_VDPAU_STATE_USED_FOR_REFERENCE;
    pic->reordered_opaque= avctx->reordered_opaque;
    if(pic->reference)
    {
        pic->age=ip_age[0];
        ip_age[0]=ip_age[1]+1;
        ip_age[1]=1;
        b_age++;
    }else
    {
        pic->age=b_age;
        ip_age[0]++;
        ip_age[1]++;
        b_age=1;
    }
    return 0;
}
/**
    \fn releaseBuffer
*/
void decoderFFVDPAU::releaseBuffer(AVCodecContext *avctx, AVFrame *pic)
{
  vdpau_render_state * render;
  int i;

  render=(vdpau_render_state*)pic->data[0];
  ADM_assert(render);

  render->state &= ~FF_VDPAU_STATE_USED_FOR_REFERENCE;
  for(i=0; i<4; i++){
    pic->data[i]= NULL;
  }
  VDPAU->freeQueue.push_back(render);
}
/**
    \fn ADM_VDPAUreleaseBuffer
*/
 void ADM_VDPAUreleaseBuffer(struct AVCodecContext *avctx, AVFrame *pic)
{
    decoderFFVDPAU *dec=(decoderFFVDPAU *)avctx->opaque;
    dec->releaseBuffer(avctx,pic);
}
/**
    \fn decoderFFVDPAU
*/
decoderFFVDPAU::decoderFFVDPAU(uint32_t w, uint32_t h, uint32_t l, uint8_t * d):decoderFF (w,	   h)
{
        _context->opaque          = this;
        _context->get_buffer      = ADM_VDPAUgetBuffer;
        _context->release_buffer  = ADM_VDPAUreleaseBuffer;
        _context->draw_horiz_band = draw;
        _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
        _context->extradata = (uint8_t *) d;
        _context->extradata_size = (int) l;

        vdpau=(void *)new vdpauContext;
        VDPAU->vdpDecoder=VDP_INVALID_HANDLE;
        WRAP_OpenByName(h264_vdpau,CODEC_ID_H264);
        
        // Now instantiate our VDPAU surface & decoder
        ADM_assert(VDP_STATUS_OK==funcs.decoderCreate(vdpDevice,VDP_DECODER_PROFILE_H264_HIGH,w,h,15,&(VDPAU->vdpDecoder)));
        // Create our surfaces...
        for(int i=0;i<NB_SURFACE;i++)
        {
            VDPAU->renders[i]=new vdpau_render_state;
            ADM_assert(VDP_STATUS_OK==funcs.createSurface(vdpDevice,VDP_CHROMA_TYPE_420,w,h,&(VDPAU->renders[i]->surface)));
            VDPAU->freeQueue.push_back(VDPAU->renders[i]);
        }
        scratch=new ADMImage(w,h,1);
        b_age = ip_age[0] = ip_age[1] = 256*256*256*64;

}
/**
    \fn ~            void    goOn( const AVFrame *d);
*/
decoderFFVDPAU::~decoderFFVDPAU()
{
        printf("[VDPAU] Cleaning up\n");
        for(int i=0;i<NB_SURFACE;i++)
        {
            ADM_assert(VDP_STATUS_OK==funcs.destroySurface((VDPAU->renders[i]->surface)));
            delete VDPAU->renders[i];
        }
         ADM_assert(VDP_STATUS_OK==funcs.decoderDestroy(VDPAU->vdpDecoder));
         delete VDPAU;
         vdpau=NULL;
}
/**
    \fn uncompress
*/
uint8_t decoderFFVDPAU::uncompress (ADMCompressedImage * in, ADMImage * out)
{
VdpStatus status;
    
    // First let ffmpeg prepare datas...
    vdpau_copy=out;
    decode_status=false;
    if(!decoderFF::uncompress (in, scratch))
    {
        printf("[VDPAU] No data from libavcodec\n");
        return 0;
    }
    if(decode_status!=true)
    {
        printf("[VDPAU] error in renderDecode\n");
        return 0;
    }
    // other part will be done in goOn
  struct vdpau_render_state *rndr = (struct vdpau_render_state *)scratch->_planes[0];
   VdpVideoSurface  surface;

    surface=rndr->surface;
 void *planes[3];
            planes[0]=vdpau_copy->GetWritePtr(PLANAR_Y);
            planes[1]=vdpau_copy->GetWritePtr(PLANAR_U);
            planes[2]=vdpau_copy->GetWritePtr(PLANAR_V);
    uint32_t stride[3];
            stride[0]=vdpau_copy->GetPitch(PLANAR_Y);
            stride[1]=vdpau_copy->GetPitch(PLANAR_U);
            stride[2]=vdpau_copy->GetPitch(PLANAR_V);

    
   // Copy back the decoded image to our output ADM_image
   aprintf("[VDPAU] Getting datas from surface %d\n",surface);
    status=funcs.getDataSurface(
                surface,
                VDP_YCBCR_FORMAT_YV12, //VdpYCbCrFormat   destination_ycbcr_format,
                planes, //void * const *   destination_data,
                stride //destination_pitches
                );
    if(VDP_STATUS_OK!=status)
    {
        
        printf("[VDPAU] Cannot get data from surface <%s>\n",funcs.getErrorString(status));
        decode_status=false;
        return 0 ;
    }
    

    //
    out->Pts=scratch->Pts;
    out->flags=scratch->flags;
    return (uint8_t)decode_status;
}
/**
    \fn goOn
    \brief Callback from ffmpeg when a pic is ready to be decoded
*/
void decoderFFVDPAU::goOn( const AVFrame *d,int type)
{
   VdpStatus status;
   struct vdpau_render_state *rndr = (struct vdpau_render_state *)d->data[0];
   VdpVideoSurface  surface;

    surface=rndr->surface;
    vdpau_pts=d->reordered_opaque; // Retrieve our PTS

     aprintf("[VDPAU] Decoding Using surface %d\n", surface);
    status=funcs.decoderRender(VDPAU->vdpDecoder, surface,
                            (void * const *)&rndr->info, rndr->bitstream_buffers_used, rndr->bitstream_buffers);
    if(VDP_STATUS_OK!=status)
    {
        printf("[VDPAU] No data after decoderRender <%s>\n",funcs.getErrorString(status));
        decode_status=false;
        return ;
    }
    aprintf("[VDPAU] DecodeRender Ok***\n");
    decode_status=true;
    return;
}


/**
    \fn draw
    \brief callback invoked by lavcodec when a pic is ready to be decoded
*/
void draw(struct AVCodecContext *s,    const AVFrame *src, int offset[4],    int y, int type, int height)
{
    decoderFFVDPAU *dec=(decoderFFVDPAU *)s->opaque;
    dec->goOn(src,type);
}

#endif
// EOF
