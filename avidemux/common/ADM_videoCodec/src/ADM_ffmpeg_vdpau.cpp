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
#include "libavcodec/internal.h"
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

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

typedef enum 
{
    ADM_VDPAU_INVALID=0,
    ADM_VDPAU_H264=1,
    ADM_VDPAU_MPEG2=2,
    ADM_VDPAU_VC1=3
}ADM_VDPAU_TYPE;

#ifdef USE_VDPAU
#include "ADM_ffmpeg_vdpau_utils.cpp"
#endif


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
    \fn getBuffer
    \brief returns a VDPAU render masquerading as a AVFrame
*/
int decoderFFVDPAU::getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
    vdpau_render_state * render=NULL;
    if(VDPAU->freeQueue.size()==0)
    {
        ADM_info("[VDPAU] No more available surface, creating a new one\n");
        render=new vdpau_render_state;
        memset(render,0,sizeof( vdpau_render_state));
        int widthToUse = (avctx->coded_width+ 1)  & ~1;
        int heightToUse= (avctx->coded_height+3)  & ~3;
        if(VDP_STATUS_OK!=admVdpau::surfaceCreate(widthToUse,heightToUse,&(render->surface)))
        {
            ADM_error("Cannot create surface \n");
            delete render;
            alive=false;
            return -1;
        }
        render->state=0;
        vdpauMarkSurfaceUsed(VDPAU,(void *)render);
        surfaceMutex.lock();
        VDPAU->fullQueue.push_back(render);
        surfaceMutex.unlock();
    }else
    {
        surfaceMutex.lock();
        render=VDPAU->freeQueue.front();
        ADM_assert(render);
        VDPAU->freeQueue.erase(VDPAU->freeQueue.begin());
        surfaceMutex.unlock();
    }
    render->refCount=0;
    render->state=0;  
    vdpauMarkSurfaceUsed(VDPAU,(void *)render); // it is out of the queue, no need to lock it
    // It only works because surface is the 1st field of render!
    pic->buf[0]=av_buffer_create((uint8_t *)&(render->surface),  // Maybe a memleak here...
                                     sizeof(render->surface),
                                     ADM_VDPAUreleaseBuffer, 
                                     (void *)this,
                                     AV_BUFFER_FLAG_READONLY);

    pic->data[0]=(uint8_t *)render;
    pic->data[3]=(uint8_t *)(uintptr_t)render->surface;
    pic->reordered_opaque= avctx->reordered_opaque;
    aprintf("Alloc ===> Got buffer = %d\n",render->surface);
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
  aprintf("Release ===> Release buffer = %d\n",rdr->surface);
  vdpauMarkSurfaceUnused(VDPAU,(void *)rdr);
}

extern "C"
{

static enum AVPixelFormat vdpauGetFormat(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt)
{
    int i;
    ADM_info("VDPAU: GetFormat\n");
    AVCodecID id=AV_CODEC_ID_NONE;
    AVPixelFormat c;
    AVPixelFormat outPix;
    for(i=0;fmt[i]!=AV_PIX_FMT_NONE;i++)
    {
        c=fmt[i];
        ADM_info("VDPAU: Evaluating %d\n",c);
        if(c!=AV_PIX_FMT_VDPAU) continue;
#define FMT_V_CHECK(x,y)      case AV_CODEC_ID_##x:   outPix=AV_PIX_FMT_VDPAU_##y;id=avctx->codec_id;break;
        switch(avctx->codec_id)
        {
            FMT_V_CHECK(H264,H264)
            FMT_V_CHECK(MPEG1VIDEO,MPEG1)
            FMT_V_CHECK(MPEG2VIDEO,MPEG2)
            FMT_V_CHECK(WMV3,WMV3)
            FMT_V_CHECK(VC1,VC1)
            default: 
                continue;
                break;
        }
        break;
    }
    if(id==AV_CODEC_ID_NONE)
    {
        return AV_PIX_FMT_NONE;
    }
    // Finish intialization of Vdpau decoder
    const AVHWAccel *accel=parseHwAccel(outPix,id);
    if(accel)
    {
        ADM_info("Found matching hw accelerator : %s\n",accel->name);
        ADM_info("Successfully setup hw accel\n");
        return AV_PIX_FMT_VDPAU;
    }
    return AV_PIX_FMT_NONE;
#endif
}
}

/**
 * 
 * @return 
 */
bool decoderFFVDPAU::initVdpContext()
{
       ADM_assert (_context);
        _context->max_b_frames = 0;
        _context->width =  _w;
        _context->height = _h;
        _context->pix_fmt = AV_PIX_FMT_YUV420P;
        _context->debug_mv |= FF_SHOW;
        _context->debug |= FF_DEBUG_VIS_MB_TYPE*0 + FF_DEBUG_VIS_QP*0;
        _context->workaround_bugs=1*FF_BUG_AUTODETECT +0*FF_BUG_NO_PADDING; 
        _context->error_concealment=3; 
        if (_setFcc) 
        {
          _context->codec_tag=_fcc; 
        }
        if (_extraDataCopy) 
        {
          _context->extradata = _extraDataCopy;
          _context->extradata_size = _extraDataLen;
        }        
        _context->thread_count    =0;        
        _context->opaque          = this;
        _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
    
       
        AVVDPAUContext *v= av_alloc_vdpaucontext();;
        _context->hwaccel_context = v;
        v->render=NULL;
        v->decoder=VDP_INVALID_HANDLE; 
        
        if(0>av_vdpau_bind_context(_context, (VdpDevice)(uint64_t) admVdpau::getVdpDevice(),
                          vdpGetProcAddressWrapper, AV_HWACCEL_FLAG_IGNORE_LEVEL))
        {
             ADM_error("Cannot bind\n");
                return false;
        }
        ADM_info("Init VDP context ok\n");
        return true;
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
        ADM_VDPAU_TYPE vdpauType=ADM_VDPAU_INVALID;
        AVCodecID codecID;
        int vdpDecoder=0;
        const char *name="";
        VdpDevice dev=(VdpDevice)(uint64_t)admVdpau::getVdpDevice();
        
        if(isH264Compatible(fcc))
        {
            vdpauType=ADM_VDPAU_H264;
            name="h264";
            codecID=AV_CODEC_ID_H264;  
            vdpDecoder=VDP_DECODER_PROFILE_H264_HIGH;
        }else if(isMpeg12Compatible(fcc))
        {
            vdpauType=ADM_VDPAU_MPEG2;
            vdpDecoder=VDP_DECODER_PROFILE_MPEG2_MAIN;
            name="mpegvideo";
            codecID=AV_CODEC_ID_MPEG2VIDEO;            
        
        }else if(isVC1Compatible(fcc))
        {
            vdpauType=ADM_VDPAU_VC1;
            name="vc1";
            codecID=AV_CODEC_ID_VC1;
            vdpDecoder=VDP_DECODER_PROFILE_VC1_ADVANCED;
        }else ADM_assert(0);
       
        AVCodec *codec=avcodec_find_decoder_by_name(name);
        if(!codec) 
                {GUI_Error_HIG("Codec",QT_TR_NOOP("Internal error finding codecd\n"));ADM_assert(0);}
        codecId=codecID; 
        _context = avcodec_alloc_context3 (codec);
     
        if(!initVdpContext())
        {
            ADM_warning("Cannot setup context\n");
            alive=false;
            return ;
        }
        
        if (avcodec_open2(_context, codec, NULL) < 0)  
        { 
            printf("[lavc] Decoder init: video decoder failed!\n"); 
            GUI_Error_HIG("Codec","Internal error opening " ); 
            ADM_assert(0); 
        } 
        else 
        { 
            printf("[lavc] Decoder init: " " video decoder initialized! (%s)\n",codec->long_name); 
        }
        
        _context->get_format      = vdpauGetFormat;
        _context->get_buffer2     = ADM_VDPAUgetBuffer;
        _context->draw_horiz_band = NULL;
        _context->slice_flags     =0;        
        ADM_info("Successfully setup hw accel\n");              
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
            if(_context->hwaccel_context)
            {
                av_freep(&_context->hwaccel_context);
                _context->hwaccel_context=NULL;
            }
            avcodec_close (_context);
            av_free(_context);
            _context=NULL;
        }
        // Delete free only, the ones still used will be deleted later
        int n=VDPAU->fullQueue.size();        
        for(int i=0;i<n;i++)
        {
            vdpau_render_state *r=VDPAU->fullQueue[i];
            if(r)
            {
                if(r->surface)
                {
                        if(VDP_STATUS_OK!=admVdpau::surfaceDestroy((r->surface)))
                                ADM_error("Error destroying surface %d\n",i);
                }
                delete r;
            }
            VDPAU->fullQueue.clear();
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
    aprintf("==> uncompress %s\n",_context->codec->long_name);
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
        out->Pts= (uint64_t)(_frame->reordered_opaque);
        ADM_info("[VDPAUDec] No picture \n");
        return false;
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
    aprintf("Decoding ===> Got surface = %d\n",rndr->surface);
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

// EOF
