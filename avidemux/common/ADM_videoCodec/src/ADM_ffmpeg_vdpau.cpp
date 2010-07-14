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
 #include "ADM_ffmpeg/libavcodec/vdpau.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_render/GUI_render.h"
#include "ADM_ffmpeg_vdpau_internal.h"
#include "prefs.h"
#include "ADM_coreVdpau/include/ADM_coreVdpau.h"
#include "ADM_codecVdpau.h"



static bool vdpauWorking=false;


#define aprintf(...) {}

typedef enum 
{
    ADM_VDPAU_INVALID=0,
    ADM_VDPAU_H264=1,
    ADM_VDPAU_MPEG2=2,
    ADM_VDPAU_VC1=3
}ADM_VDPAU_TYPE;

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
    \fn vdpauProbe
    \brief Try loading vdpau...
*/
bool vdpauProbe(void)
{
    GUI_WindowInfo xinfo;
    void *draw;
    draw=UI_getDrawWidget();
    UI_getWindowInfo(draw,&xinfo );
    if(false==admVdpau::init(&xinfo)) return false;
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
        aprintf("[VDPAU] No more available surface\n");
        return -1;
    }
    // Get an image   
    render=VDPAU->freeQueue.front();
    VDPAU->freeQueue.erase(VDPAU->freeQueue.begin());
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
  if(destroying==true) return; // They are already freed...
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
    \fn vdpauGetFormat
    \brief Borrowed from mplayer

*/
extern "C"
{
static enum PixelFormat vdpauGetFormat(struct AVCodecContext *avctx,  const enum PixelFormat *fmt)
{
    int i;

    for(i=0;fmt[i]!=PIX_FMT_NONE;i++)
    {
        PixelFormat c=fmt[i];
        switch(c)
        {
            case PIX_FMT_VDPAU_H264:
            case PIX_FMT_VDPAU_MPEG1:
            case PIX_FMT_VDPAU_MPEG2:
            case PIX_FMT_VDPAU_WMV3:
            case PIX_FMT_VDPAU_VC1:
                        return c;
            default:break;

        }
    }
    return PIX_FMT_NONE;
}
}
/**
    \fn decoderFFVDPAU
*/
decoderFFVDPAU::decoderFFVDPAU(uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, 
        uint8_t *extraData,uint32_t bpp)
:decoderFF (w,h,fcc,extraDataLen,extraData,bpp)
{
        destroying=false;
        alive=true;
        scratch=NULL;
        _context->opaque          = this;
        _context->get_buffer      = ADM_VDPAUgetBuffer;
        _context->release_buffer  = ADM_VDPAUreleaseBuffer;
        _context->draw_horiz_band = draw;
        _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
        _context->extradata = (uint8_t *) extraData;
        _context->extradata_size  = (int) extraDataLen;
        _context->get_format      = vdpauGetFormat;
        vdpau=(void *)new vdpauContext;
        VDPAU->vdpDecoder=VDP_INVALID_HANDLE;
        ADM_VDPAU_TYPE vdpauType=ADM_VDPAU_INVALID;
        if(isH264Compatible(fcc))
        {
            vdpauType=ADM_VDPAU_H264;
        }else if(isMpeg12Compatible(fcc))
        {
            vdpauType=ADM_VDPAU_MPEG2;
        }else if(isVC1Compatible(fcc))
        {
            vdpauType=ADM_VDPAU_VC1;
        }else ADM_assert(0);
        int vdpDecoder;
        switch(vdpauType)
        {
            case ADM_VDPAU_VC1  : vdpDecoder=VDP_DECODER_PROFILE_VC1_ADVANCED;
                                  WRAP_OpenByName(vc1_vdpau,CODEC_ID_VC1);
                                  break;

            case ADM_VDPAU_H264 : vdpDecoder=VDP_DECODER_PROFILE_H264_HIGH;
                                  WRAP_OpenByName(h264_vdpau,CODEC_ID_H264);
                                  break;
            case ADM_VDPAU_MPEG2: 
                                  vdpDecoder=VDP_DECODER_PROFILE_MPEG2_MAIN;
                                  WRAP_OpenByName(mpegvideo_vdpau,CODEC_ID_MPEG2VIDEO);
                                  break;
            default: ADM_assert(0);
        }
        ADM_info("[VDPAU] Decoder created \n");
        // Now instantiate our VDPAU surface & decoder
        for(int i=0;i<NB_SURFACE;i++)
            VDPAU->renders[i]=NULL;
        
        if(VDP_STATUS_OK!=admVdpau::decoderCreate(vdpDecoder, w,h,15,&(VDPAU->vdpDecoder)))
        {
            ADM_error("Cannot create VDPAU decoder\n");
            alive=false;
            return;
        }
        // Create our surfaces...
        for(int i=0;i<NB_SURFACE;i++)
        {
            VDPAU->renders[i]=new vdpau_render_state;
            memset(VDPAU->renders[i],0,sizeof( vdpau_render_state));
            if(VDP_STATUS_OK!=admVdpau::surfaceCreate(w,h,&(VDPAU->renders[i]->surface)))
            {
                ADM_error("Cannot create surface %d/%d\n",i,NB_SURFACE);
                alive=false;
                return;
            }
            VDPAU->freeQueue.push_back(VDPAU->renders[i]);
        }
        scratch=new ADMImageRef(w,h);
        b_age = ip_age[0] = ip_age[1] = 256*256*256*64;

}
/**
    \fn ~            void    goOn( const AVFrame *d);
*/
decoderFFVDPAU::~decoderFFVDPAU()
{
        ADM_info("[VDPAU] Cleaning up\n");
        destroying=true;
        for(int i=0;i<NB_SURFACE;i++)
        {
            if(VDPAU->renders[i])
            {
                if(VDPAU->renders[i]->surface)
                {
                    if(VDPAU->renders[i]->surface)
                        if(VDP_STATUS_OK!=admVdpau::surfaceDestroy((VDPAU->renders[i]->surface)))
                                ADM_error("Error destroying surface %d\n",i);
                }
                delete VDPAU->renders[i];
            }
        }
         ADM_info("[VDPAU] Destroying decoder\n");
         if(VDP_STATUS_OK!=admVdpau::decoderDestroy(VDPAU->vdpDecoder))
                ADM_error("Error destroying VDPAU decoder\n");
         delete VDPAU;
         vdpau=NULL;
}
/**
    \fn uncompress
*/
bool decoderFFVDPAU::uncompress (ADMCompressedImage * in, ADMImage * out)
{
VdpStatus status;
    
    // First let ffmpeg prepare datas...
    vdpau_copy=out;
    decode_status=false;
    if(!decoderFF::uncompress (in, scratch))
    {
        aprintf("[VDPAU] No data from libavcodec\n");
        return 0;
    }
    if(decode_status!=true)
    {
        printf("[VDPAU] error in renderDecode\n");
        return 0;
    }
    // other part will be done in goOn
   struct vdpau_render_state *rndr = (struct vdpau_render_state *)scratch->GetReadPtr(PLANAR_Y);
   VdpVideoSurface  surface;

    surface=rndr->surface;
    uint8_t *planes[3];
    uint32_t stride[3];
             vdpau_copy->GetWritePlanes(planes);
             vdpau_copy->GetPitches(stride);

    
   // Copy back the decoded image to our output ADM_image
   aprintf("[VDPAU] Getting datas from surface %d\n",surface);
    status=admVdpau::getDataSurface(surface,planes, stride );
    if(VDP_STATUS_OK!=status)
    {
        
        printf("[VDPAU] Cannot get data from surface <%s>\n",admVdpau::getErrorString(status));
        decode_status=false;
        return 0 ;
    }
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
    status=admVdpau::decoderRender(VDPAU->vdpDecoder, surface,
                            &rndr->info, rndr->bitstream_buffers_used, rndr->bitstream_buffers);
    if(VDP_STATUS_OK!=status)
    {
        printf("[VDPAU] No data after decoderRender <%s>\n",admVdpau::getErrorString(status));
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
