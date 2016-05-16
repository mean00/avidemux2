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

#if 1
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

/**
    \fn vdpauGetFormat
    \brief Borrowed from mplayer/ffmpeg
 
 

*/

static const AVHWAccel *parseHwAccel(enum AVPixelFormat pix_fmt,AVCodecID id)
{
    AVHWAccel *hw=av_hwaccel_next(NULL);
    
    while(hw)
    {
        ADM_info("Trying %s, %d : %d, codec =%d : %d\n",hw->name,hw->pix_fmt,pix_fmt,hw->id,id);
        if (hw->pix_fmt == AV_PIX_FMT_VDPAU && id==hw->id)
            return hw;
        hw=av_hwaccel_next(hw);
    }
    return NULL;
}

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

extern "C"
{

static enum AVPixelFormat vdpauGetFormat(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt)
{
    int i;
    ADM_info("VDPAU: GetFormat\n");
    AVPixelFormat found=AV_PIX_FMT_NONE;
    AVCodecID id=AV_CODEC_ID_NONE;
    AVPixelFormat c;
    for(i=0;fmt[i]!=AV_PIX_FMT_NONE;i++)
    {
        c=fmt[i];
        ADM_info("VDPAU: Evaluating %d\n",c);
        
        found=c;
        switch(c)
        {
            case AV_PIX_FMT_VDPAU_H264: id=AV_CODEC_ID_H264;break;
            case AV_PIX_FMT_VDPAU_MPEG1:id=AV_CODEC_ID_MPEG1VIDEO;break;
            case AV_PIX_FMT_VDPAU_MPEG2:id=AV_CODEC_ID_MPEG2VIDEO;break;
            case AV_PIX_FMT_VDPAU_WMV3: id=AV_CODEC_ID_WMV3;break;
            case AV_PIX_FMT_VDPAU_VC1:  id=AV_CODEC_ID_VC1;break;
            default: 
                    found=AV_PIX_FMT_NONE;
                    continue;
                    break;
        }
        break;
    }
    if(found==AV_PIX_FMT_NONE)
        return AV_PIX_FMT_NONE;
    // Finish intialization of Vdpau decoder
    const AVHWAccel *accel=parseHwAccel(c,id);
    if(accel)
    {
        ADM_info("Found matching hw accelerator : %s\n",accel->name);
        if(!avctx->internal->hwaccel_priv_data)
        {
            // something fishy here : setup_hwaccel is not called due to it being vdpau, so no one is doing it ....
            // I missed something obviously.
            avctx->internal->hwaccel_priv_data = av_mallocz(accel->priv_data_size); // dafuq ?
        }
        avctx->sw_pix_fmt = AV_PIX_FMT_YUV420P; // dafuq 2 ?
        int r=accel->init(avctx);
        if(r<0)
        {
            ADM_warning("Error initializing hw accel (%d)\n",r);
            return AV_PIX_FMT_NONE;
        }                         
        ADM_info("Successfully setup hw accel\n");
        return c;
    }
    return AV_PIX_FMT_NONE;
#endif
}
}

extern "C"
{
/**
 * 
 * @param s
 * @param f
 * @param info
 * @param count
 * @param buffers
 * @return 
 */
static int render2Wrapper(AVCodecContext *ctx, AVFrame *frame, const VdpPictureInfo *info, uint32_t count, const VdpBitstreamBuffer *buffers)
{
    /**
    mp_image_t *mpi = f->opaque;
    sh_video_t *sh = s->opaque;
    struct vdpau_frame_data data;
    uint8_t *planes[4] = {(void *)&data};
    data.surface = (VdpVideoSurface)mpi->priv;
    data.info = info;
    data.bitstream_buffers_used = count;
    data.bitstream_buffers = buffers;
    mpcodecs_draw_slice(sh, planes, NULL, sh->disp_w, sh->disp_h, 0, 0);
    */
            
    ADM_info("render2Wrapper called \n");
    return 0;
}

    
    
    /**
     * \fn vdpGetProcAddressWrapper
     * @param device
     * @param function_id
     * @param function_pointer
     * @return 
     */
static VdpStatus vdpGetProcAddressWrapper(    VdpDevice device,    VdpFuncId function_id,       void * *  function_pointer)
{
    ADM_info("Calling vdpGetProcAddressWrapper for function %d\n",function_id);
    VdpStatus r=VDP_STATUS_ERROR;
    
#define ADD_WRAPPER(x,y)     case VDP_FUNC_ID_##x: ADM_info("Wrapping "#x" "#y"\n"); *function_pointer= (void *)admVdpau::y;\
                                        r=VDP_STATUS_OK;\
                                        break;        
    
    switch(function_id)
    {
        ADD_WRAPPER(DECODER_CREATE,decoderCreate)
        ADD_WRAPPER(DECODER_RENDER,decoderRender)
        ADD_WRAPPER(DECODER_DESTROY,decoderDestroy)
                
        case VDP_FUNC_ID_VIDEO_SURFACE_QUERY_CAPABILITIES:
        case VDP_FUNC_ID_DECODER_QUERY_CAPABILITIES:            
        default:
            VdpGetProcAddress *p=admVdpau::getProcAddress2();
            ADM_assert(p);
            r= p(device,function_id,function_pointer);
    }
    if(r==VDP_STATUS_OK)
    {
        ADM_info("Ok\n");
    }else
    {
        ADM_warning("Failed with er=%d\n",(int)r);
    }
    return r;
    
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
        if (_setFcc) {
          _context->codec_tag=_fcc; 
        }
        if (_extraDataCopy) {
          _context->extradata = _extraDataCopy;
          _context->extradata_size = _extraDataLen;
        }        
        _context->thread_count    =0;        
        _context->opaque          = this;
        _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
    
       
        AVVDPAUContext *v= av_alloc_vdpaucontext();;
        _context->hwaccel_context = v;
        v->render2=render2Wrapper;
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
        VDPAU->vdpDecoder=VDP_INVALID_HANDLE;
        ADM_VDPAU_TYPE vdpauType=ADM_VDPAU_INVALID;
        AVCodecID codecID;
        int vdpDecoder=0;
        const char *name="";
        VdpDevice dev=(VdpDevice)(uint64_t)admVdpau::getVdpDevice();
        
        if(isH264Compatible(fcc))
        {
            vdpauType=ADM_VDPAU_H264;
            name="h264_vdpau";
            codecID=AV_CODEC_ID_H264;  
            vdpDecoder=VDP_DECODER_PROFILE_H264_HIGH;
        }else if(isMpeg12Compatible(fcc))
        {
            vdpauType=ADM_VDPAU_MPEG2;
            vdpDecoder=VDP_DECODER_PROFILE_MPEG2_MAIN;
            name="mpegvideo_vdpau";
            codecID=AV_CODEC_ID_MPEG2VIDEO;            
        
        }else if(isVC1Compatible(fcc))
        {
            vdpauType=ADM_VDPAU_VC1;
            name="vc1_vdpau";
            codecID=AV_CODEC_ID_VC1;
            vdpDecoder=VDP_DECODER_PROFILE_VC1_ADVANCED;
        }else ADM_assert(0);
        
       
        // Now instantiate our VDPAU surface & decoder
        for(int i=0;i<NB_SURFACE;i++)
            VDPAU->renders[i]=NULL;
       
        int widthToUse=admVdpau::dimensionRoundUp(w);
        int heightToUse=admVdpau::dimensionRoundUp(h);

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
