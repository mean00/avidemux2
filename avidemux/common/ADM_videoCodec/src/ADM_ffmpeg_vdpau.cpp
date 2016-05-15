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
#include "ADM_cpp.h"
#include "BVector.h"
#include "ADM_default.h"

#ifdef USE_VDPAU
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/vdpau.h"
#include "libavutil/buffer.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_render/GUI_render.h"
#include "ADM_ffmpeg_vdpau_internal.h"
#include "prefs.h"
#include "ADM_coreVideoCodec/ADM_hwAccel/ADM_coreVdpau/include/ADM_coreVdpau.h"
#include "ADM_codecVdpau.h"
#include "ADM_threads.h"

#if defined(__sun__)
#include <alloca.h>
#endif /* : defined(__sun__) */

static bool         vdpauWorking=false;
static admMutex     surfaceMutex;
#define aprintf(...) {}

typedef enum 
{
    ADM_VDPAU_INVALID=0,
    ADM_VDPAU_H264=1,
    ADM_VDPAU_MPEG2=2,
    ADM_VDPAU_VC1=3
}ADM_VDPAU_TYPE;


/**
    \fn markSurfaceUsed
    \brief mark the surfave as used. Can be called multiple time.
*/
static bool vdpauMarkSurfaceUsed(void *v, void * cookie)
{
    vdpauContext *vd=(vdpauContext*)v;
    vdpau_render_state *render=(vdpau_render_state *)cookie;
    render->refCount++;
    return true;
}
/**
    \fn markSurfaceUnused
    \brief mark the surfave as unused by the caller. Can be called multiple time.
*/
static bool vdpauMarkSurfaceUnused(void *v, void * cookie)
{
    vdpauContext *vd=(vdpauContext*)v;
    vdpau_render_state *render=(vdpau_render_state *)cookie;
    render->refCount--;
    if(!render->refCount)
    {
        surfaceMutex.lock();
        vd->freeQueue.push_back(render);
        surfaceMutex.unlock();
    }
    return true;
}
/**
    \fn vdpauRefDownload
    \brief Convert a VDPAU image to a regular image
*/

static bool vdpauRefDownload(ADMImage *image, void *instance, void *cookie)
{
    vdpauContext *vd=(vdpauContext*)instance;
    vdpau_render_state *render=(vdpau_render_state *)cookie;
    ADM_assert(render->refCount);
    ADM_assert(image->refType==ADM_HW_VDPAU);
    
    // other part will be done in goOn
    VdpVideoSurface  surface;

    surface=render->surface;
    uint8_t *planes[3];
    uint32_t stride[3];

     image->GetWritePlanes(planes);
     image->GetPitches(stride);

    //ADM_info("Getting surface %d\n",(int)surface);
    // Copy back the decoded image to our output ADM_image
    aprintf("[VDPAU] Getting datas from surface %d\n",surface);
 
    VdpStatus status=admVdpau::getDataSurface(surface,planes, stride );
    if(VDP_STATUS_OK!=status)
    {
        ADM_error("[VDPAU] Cannot get data from surface <%s>\n",admVdpau::getErrorString(status));
    }
    image->refType=ADM_HW_NONE;
    bool r=vdpauMarkSurfaceUnused(instance,cookie);
    if(r==false || status!=VDP_STATUS_OK) 
    {
        ADM_warning("Cannot get VDPAU content from surface %d\n",(int)surface);
        return false;
    }
    return true;
}

/**
    \fn vdpauUsable
    \brief Return true if  vdpau can be used...
*/
bool vdpauUsable(void)
{
    bool v=false;
    if(!vdpauWorking) return false;
    if(!prefs->get(FEATURES_VDPAU,&v)) v=false;
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
#ifdef USE_VDPAU
    if( admCoreCodecSupports(ADM_CORE_CODEC_FEATURE_VDPAU)==false)
    {
        GUI_Error_HIG("Error","Core has been compiled without VDPAU support, but the application has been compiled with it.\nInstallation mismatch");
        vdpauWorking=false;
    }
#endif
    if(false==admVdpau::init(&xinfo)) return false;
    vdpauWorking=true;
    return true;
}
/**
    \fn vdpauCleanup
*/
bool vdpauCleanup(void)
{
   return admVdpau::cleanup();
}
/**
    \fn ADM_VDPAUgetBuffer
    \brief trampoline to get a VDPAU surface
*/
int ADM_VDPAUgetBuffer(AVCodecContext *avctx, AVFrame *pic,int flags)
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
        ADM_warning("[VDPAU] No more available surface\n");
        return -1;
    }
    // Get an image   
    surfaceMutex.lock();
    render=VDPAU->freeQueue.front();
    render->refCount=0;
    VDPAU->freeQueue.erase(VDPAU->freeQueue.begin());
    surfaceMutex.unlock();
    vdpauMarkSurfaceUsed(VDPAU,(void *)render);
    
    render->state=0;
    // It only works because surface is the 1st field of render!
    pic->buf[0]=av_buffer_create((uint8_t *)&(render->surface), 
                                     sizeof(render->surface),
                                     ADM_VDPAUreleaseBuffer, 
                                     (void *)this,
                                     AV_BUFFER_FLAG_READONLY);

    pic->data[0]=(uint8_t *)render;
    pic->data[3]=(uint8_t *)(uintptr_t)render->surface;
    pic->reordered_opaque= avctx->reordered_opaque;
    return 0;
}
/**
    \fn releaseBuffer
*/
void decoderFFVDPAU::releaseBuffer(struct vdpau_render_state *rdr)
{
  ADM_assert(rdr);
  ADM_assert(rdr->refCount);
  rdr->state &= ~FF_VDPAU_STATE_USED_FOR_REFERENCE;
  vdpauMarkSurfaceUnused(VDPAU,(void *)rdr);
}
/**
    \fn ADM_VDPAUreleaseBuffer
 *  \param opaque is decoderFFVDPAU
*   \param data   is surface
 * 
*/
 void ADM_VDPAUreleaseBuffer(void *opaque, uint8_t *data)
{
      decoderFFVDPAU *dec=(decoderFFVDPAU *)opaque;
      struct vdpau_render_state *rdr=( struct vdpau_render_state  *)data;
      dec->releaseBuffer(rdr);
}
/**
    \fn vdpauGetFormat
    \brief Borrowed from mplayer

*/
extern "C"
{
static enum AVPixelFormat vdpauGetFormat(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt)
{
    int i;

    for(i=0;fmt[i]!=AV_PIX_FMT_NONE;i++)
    {
        AVPixelFormat c=fmt[i];
        switch(c)
        {
            case AV_PIX_FMT_VDPAU_H264:
            case AV_PIX_FMT_VDPAU_MPEG1:
            case AV_PIX_FMT_VDPAU_MPEG2:
            case AV_PIX_FMT_VDPAU_WMV3:
            case AV_PIX_FMT_VDPAU_VC1:
                        return c;
            default:break;

        }
    }
    return AV_PIX_FMT_NONE;
}
}
/**
    \fn decoderFFVDPAU
*/
decoderFFVDPAU::decoderFFVDPAU(uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, 
        uint8_t *extraData,uint32_t bpp)
:decoderFF (w,h,fcc,extraDataLen,extraData,bpp)
{
        alive=true;
        avVdCtx=NULL;
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
                                  WRAP_Open_TemplateVdpauByName("vc1_vdpau",AV_CODEC_ID_VC1);
                                  break;

            case ADM_VDPAU_H264 : vdpDecoder=VDP_DECODER_PROFILE_H264_HIGH;
                                  WRAP_Open_TemplateVdpauByName("h264_vdpau",AV_CODEC_ID_H264);
                                  break;
            case ADM_VDPAU_MPEG2: 
                                  vdpDecoder=VDP_DECODER_PROFILE_MPEG2_MAIN;
                                  WRAP_Open_TemplateVdpauByName("mpegvideo_vdpau",AV_CODEC_ID_MPEG2VIDEO);
                                  break;
            default: ADM_assert(0);break;
        }
        ADM_info("[VDPAU] Decoder created \n");
        // Now instantiate our VDPAU surface & decoder
        for(int i=0;i<NB_SURFACE;i++)
            VDPAU->renders[i]=NULL;
       
        int widthToUse=admVdpau::dimensionRoundUp(w);
        int heightToUse=admVdpau::dimensionRoundUp(h);
 
        if(VDP_STATUS_OK!=admVdpau::decoderCreate(vdpDecoder, widthToUse,heightToUse,15,&(VDPAU->vdpDecoder)))
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
            if(VDP_STATUS_OK!=admVdpau::surfaceCreate(widthToUse,heightToUse,&(VDPAU->renders[i]->surface)))
            {
                ADM_error("Cannot create surface %d/%d\n",i,NB_SURFACE);
                alive=false;
                return;
            }
            VDPAU->freeQueue.push_back(VDPAU->renders[i]);
        }
        
        avVdCtx=av_vdpau_alloc_context();
        avVdCtx->render=admVdpau::decoderRender;
        VdpDevice dev=(VdpDevice)(uint64_t)admVdpau::getVdpDevice();
        if (av_vdpau_bind_context(_context, 
                                    dev,
                                    (VdpGetProcAddress*)admVdpau::getProcAddress(),
                                    AV_HWACCEL_FLAG_IGNORE_LEVEL))
        {
            ADM_warning("Binding context failed\n");
            alive=false;
            return;
        }
        _context->get_buffer2=ADM_VDPAUgetBuffer;
        _context->get_format =vdpauGetFormat;


}
/**
    \fn ~            void    goOn( const AVFrame *d);
*/
decoderFFVDPAU::~decoderFFVDPAU()
{
        ADM_info("[VDPAU] Cleaning up\n");
        if(_context) // duplicate ~decoderFF to make sure in transit buffers are 
        // released
        {
                avcodec_close (_context);
                av_free(_context);
                _context=NULL;
        }
        // Delete free only, the ones still used will be deleted later
        int n=VDPAU->freeQueue.size();        
        for(int i=0;i<n;i++)
        {
            vdpau_render_state *r=VDPAU->freeQueue[i];
            if(r)
            {
                if(r->surface)
                {
                        if(VDP_STATUS_OK!=admVdpau::surfaceDestroy((r->surface)))
                                ADM_error("Error destroying surface %d\n",i);
                }
                delete r;
            }
            VDPAU->freeQueue.clear();
        }
        if (VDPAU->vdpDecoder) {
            ADM_info("[VDPAU] Destroying decoder\n");
            if(VDP_STATUS_OK!=admVdpau::decoderDestroy(VDPAU->vdpDecoder))
                ADM_error("Error destroying VDPAU decoder\n");
        }
        delete VDPAU;
        vdpau=NULL;
        // frame & hw_accel to free TODO FIXME 
}
/**
    \fn uncompress
*/
bool decoderFFVDPAU::uncompress (ADMCompressedImage * in, ADMImage * out)
{

    if(out->refType==ADM_HW_VDPAU)
    {
            vdpauMarkSurfaceUnused(VDPAU,out->refDescriptor.refCookie);
            out->refType=ADM_HW_NONE;
    }

    if (in->dataLength == 0 && !_allowNull)	// Null frame, silently skipped
    {
        out->_noPicture = 1;
        out->Pts=ADM_COMPRESSED_NO_PTS;
        ADM_info("[VDpau] Nothing to decode -> no Picture\n");
        return true;
    }

   // Put a safe value....
   out->Pts=in->demuxerPts;
    _context->reordered_opaque=in->demuxerPts;
    int got_picture;
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data=in->data;
    pkt.size=in->dataLength;
    if(in->flags&AVI_KEY_FRAME)
        pkt.flags=AV_PKT_FLAG_KEY;
    else
        pkt.flags=0;

    int ret = avcodec_decode_video2 (_context, _frame, &got_picture, &pkt);
    
    if(ret<0)
    {
        char er[2048]={0};
        av_make_error_string(er, sizeof(er)-1, ret);
        ADM_warning("Error %d in lavcodec (%s)\n",ret,er);
        return false;
    }
    if(_frame->pict_type==AV_PICTURE_TYPE_NONE)
    {
        out->_noPicture=true;
        return true;
    }
    return readBackBuffer(_frame,in,out);
}
 /**
  * \fn readBackBuffer
  * @param decodedFrame
  * @param in
  * @param out
  * @return 
  */   
bool     decoderFFVDPAU::readBackBuffer(AVFrame *decodedFrame, ADMCompressedImage * in, ADMImage * out)
{
   // VdpSurface *surface=decodedFrame->data[3];
    struct vdpau_render_state *rndr = (struct vdpau_render_state *)decodedFrame->data[0];
    ADM_assert(rndr);
    out->refType=ADM_HW_VDPAU;
    out->refDescriptor.refCookie=(void *)rndr;
    out->refDescriptor.refInstance=VDPAU;
    out->refDescriptor.refMarkUsed=vdpauMarkSurfaceUsed;
    out->refDescriptor.refMarkUnused=vdpauMarkSurfaceUnused;
    out->refDescriptor.refDownload=vdpauRefDownload;
    vdpauMarkSurfaceUsed(VDPAU,(void *)rndr);
    
    uint64_t pts_opaque=(uint64_t)(decodedFrame->reordered_opaque);
    out->Pts= (uint64_t)(pts_opaque);    
    out->flags=admFrameTypeFromLav(decodedFrame);
    return true;
}


#endif
// EOF
