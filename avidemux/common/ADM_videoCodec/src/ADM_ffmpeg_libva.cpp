/***************************************************************************
            \file              ADM_ffmpeg_libvap.cpp  
            \brief Decoder using half ffmpeg/half vaapi

 
 Very similar to ffmpeg_vdpau


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

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/hwcontext_vaapi.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_render/GUI_render.h"
#include "prefs.h"
#include "ADM_coreLibVA.h"
#include "../private_inc/ADM_codecLibVA.h"
#include "ADM_threads.h"
#include "ADM_vidMisc.h"
#include "prefs.h"


static bool         libvaWorking=true;
static bool         libvaEncoderWorking=false;
static bool         libvaDri3Disabled=false;
static admMutex     imageMutex;
static int  ADM_LIBVAgetBuffer(AVCodecContext *avctx, AVFrame *pic);
static void ADM_LIBVAreleaseBuffer(struct AVCodecContext *avctx, AVFrame *pic);



#if 1
#define aprintf(...) {}
#else
#define aprintf ADM_info
#endif

/**
 * 
 * @param instance
 * @param cookie
 * @return 
 */
static  bool libvaMarkSurfaceUsed(void *instance,void *cookie)
{
    decoderFFLIBVA *inst=(decoderFFLIBVA *)instance;
    ADM_vaSurface *s=(ADM_vaSurface *)cookie ;
    inst->markSurfaceUsed(s);
    return true;
}
/**
 * 
 * @param instance
 * @param cookie
 * @return 
 */
static  bool libvaMarkSurfaceUnused(void *instance,void *cookie)
{
    decoderFFLIBVA *inst=(decoderFFLIBVA *)instance ;
    ADM_vaSurface *s=(ADM_vaSurface *)cookie ;
    inst->markSurfaceUnused(s);
    return true;
}

/**
    \fn vdpauRefDownload
    \brief Convert a VASurface image to a regular image
*/
static bool libvaRefDownload(ADMImage *image, void *instance, void *cookie)
{
    decoderFFLIBVA *inst=(decoderFFLIBVA *)instance ;
    ADM_vaSurface *s=(ADM_vaSurface *) cookie;
    bool r=s->toAdmImage(image);
    image->refType=ADM_HW_NONE;
    libvaMarkSurfaceUnused(instance,cookie);
    return r;
}

/**
    \fn libvaUsable
    \brief Return true if  libva can be used...
*/
bool libvaUsable(void)
{    
    bool v=true;
    if(!libvaWorking) return false;
    if(!prefs->get(FEATURES_LIBVA,&v)) 
    {
        v=false;
    }
    return v;
}

static ADM_vaSurface *allocateADMVaSurface(AVCodecContext *ctx)
{
    int widthToUse = (ctx->coded_width+ 1)  & ~1;
    int heightToUse= (ctx->coded_height+3)  & ~3;
    int fmt=VA_RT_FORMAT_YUV420;
    if(ctx->pix_fmt==AV_PIX_FMT_YUV420P10LE)
        fmt=VA_RT_FORMAT_YUV420_10BPP;
    VASurfaceID surface=admLibVA::allocateSurface(widthToUse,heightToUse,fmt);
    if(surface==VA_INVALID)
    {
            ADM_warning("Cannot allocate surface (%d x %d)\n",(int)widthToUse,(int)heightToUse);
            return NULL;
    }
    ADM_vaSurface *img=new ADM_vaSurface(widthToUse,heightToUse);
    img->surface=surface;
    return img;
}
/**
    \fn libvaProbe
    \brief Try loading vaapi...
*/
extern bool ADM_initLibVAEncoder(void);
bool libvaProbe(void)
{
    ADM_info("Probing for libVA support...\n");
    GUI_WindowInfo xinfo;
    void *draw;
    draw=UI_getDrawWidget();
    UI_getWindowInfo(draw,&xinfo );
    if( admCoreCodecSupports(ADM_CORE_CODEC_FEATURE_LIBVA)==false)
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Error"),QT_TRANSLATE_NOOP("adm","Core has been compiled without LIBVA support, but the application has been compiled with it.\nInstallation mismatch"));
        libvaWorking=false;
        return false;
    }

    // Needed to unbreak vaPutSurface() with libva >= 2.17, see https://github.com/intel/libva/pull/679
    if(!getenv("LIBVA_DRI3_DISABLE"))
    {
        ADM_info("LIBVA_DRI3_DISABLE not set yet, doing it now.\n");
        if(setenv("LIBVA_DRI3_DISABLE", "1", 1))
            ADM_warning("Cannot set LIBVA_DRI3_DISABLE env var\n");
        else
            libvaDri3Disabled = true;
    }else
    {
        ADM_info("LIBVA_DRI3_DISABLE already set.\n");
    }

    if(false == admLibVA::init(&xinfo))
    {
        if(libvaDri3Disabled)
            unsetenv("LIBVA_DRI3_DISABLE");
        libvaDri3Disabled = false;
        return false;
    }
    libvaWorking=true;
    { // Only check encoder if decoder is working
       libvaEncoderWorking = ADM_initLibVAEncoder();
        
    }
    if(libvaEncoderWorking) 
        ADM_info("LIBVA HW encoder is working\n");
    else 
        ADM_info("LIBVA HW encoder is *NOT* working\n");
    return true;
}

/**
    \fn markSurfaceUsed
    \brief mark the surfave as used. Can be called multiple time.
*/
bool decoderFFLIBVA::markSurfaceUsed(ADM_vaSurface *s)
{
    imageMutex.lock();
    s->refCount++;
    imageMutex.unlock();
    return true;
    
}
bool        decoderFFLIBVA::markSurfaceUnused(VASurfaceID id)
{
    aprintf("Freeing surface %x\n",(int)id);
    ADM_vaSurface *s=lookupBySurfaceId(id);
    ADM_assert(s);
    return markSurfaceUnused(s);
}
/**
    \fn markSurfaceUnused
    \brief mark the surfave as unused by the caller. Can be called multiple time.
*/
bool decoderFFLIBVA::markSurfaceUnused(ADM_vaSurface *img)
{
        
   imageMutex.lock();
   img->refCount--;
   aprintf("Surface %x, Ref count is now %d\n",img->surface,img->refCount);
   if(!img->refCount)
   {
        vaPool.freeSurfaceQueue.append(img);
   }
   imageMutex.unlock();
   return true;
}
 
/**
    \fn ADM_LIBVAgetBuffer
    \brief trampoline to get a LIBVA surface
*/
int ADM_LIBVAgetBuffer(AVCodecContext *avctx, AVFrame *pic,int flags)
{
    decoderFF *ff=(decoderFF *)avctx->opaque;
    decoderFFLIBVA *dec=(decoderFFLIBVA *)ff->getHwDecoder();
    ADM_assert(dec);
    return dec->getBuffer(avctx,pic);
}

/**
 * \fn ADM_XVBAreleaseBuffer
 * @param avctx
 * @param pic
 */
void ADM_LIBVAreleaseBuffer(void *opaque, uint8_t *data)
{
   decoderFFLIBVA *dec=(decoderFFLIBVA *)opaque;
   ADM_assert(dec);
   VASurfaceID   surface=*(VASurfaceID *)(data);
   dec->markSurfaceUnused(surface);
}

/**
 * 
 * @param avctx
 * @param pic
 * @return 
 */
int decoderFFLIBVA::getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
        
    imageMutex.lock();
    if(vaPool.freeSurfaceQueue.empty())
    {
        aprintf("Allocating new vaSurface\n");
        ADM_vaSurface *img=allocateADMVaSurface(avctx);
        if(!img)
        {
            imageMutex.unlock();
            ADM_warning("Cannot allocate new vaSurface!\n");
            return -1;
        }
        vaPool.freeSurfaceQueue.append(img);
        vaPool.allSurfaceQueue.append(img);
    }else
    {
        aprintf("Reusing vaSurface from pool\n");
    }
    ADM_vaSurface *s= vaPool.freeSurfaceQueue[0];
    vaPool.freeSurfaceQueue.popFront();
    imageMutex.unlock();
    s->refCount=0;
    markSurfaceUsed(s); // 1 ref taken by lavcodec
    
    pic->buf[0]=av_buffer_create((uint8_t *)&(s->surface),  // Maybe a memleak here...
                                     sizeof(s->surface),
                                     ADM_LIBVAreleaseBuffer, 
                                     (void *)this,
                                     AV_BUFFER_FLAG_READONLY);
    
    aprintf("Alloc Buffer : 0x%llx, surfaceid=%x\n",s,(int)s->surface);
    pic->data[0]=(uint8_t *)s;
    pic->data[3]=(uint8_t *)(uintptr_t)s->surface;
    pic->reordered_opaque= avctx->reordered_opaque;    
    return 0;
}

/**
    \fn admLibVa_exitCleanup
*/
bool admLibVa_exitCleanup(void)
{
    if(libvaDri3Disabled)
    {
        ADM_info("LIBVA_DRI3_DISABLE set by us, unsetting\n");
        unsetenv("LIBVA_DRI3_DISABLE");
    }
    libvaDri3Disabled = false;
    return admLibVA::cleanup();
}


/**
 * \fn lookupBySurfaceId
 * @param 
 * @return 
 */
ADM_vaSurface *decoderFFLIBVA::lookupBySurfaceId(VASurfaceID id)
{
    aprintf("Looking up surface %x\n",(int)id);
    imageMutex.lock();
    int n=vaPool.allSurfaceQueue.size();
    for(int i=0;i<n;i++)
        if(vaPool.allSurfaceQueue[i]->surface==id)
        {
            imageMutex.unlock();
            return vaPool.allSurfaceQueue[i];
        }
    imageMutex.unlock();
    ADM_warning("Lookup a non existing surface\n");
    ADM_assert(0);
    return NULL;
    
}
  




extern "C"
{

static enum AVPixelFormat ADM_LIBVA_getFormat(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt)
{
    int i;
    ADM_info("[LIBVA]: GetFormat\n");
    AVCodecID id=AV_CODEC_ID_NONE;
    AVPixelFormat c;
    AVPixelFormat outPix;
    for(i=0;fmt[i]!=AV_PIX_FMT_NONE;i++)
    {
        c=fmt[i];
        char name[300]={0};
        av_get_pix_fmt_string(name,sizeof(name),c);
        ADM_info("[LIBVA]: Evaluating PIX_FMT %d,%s\n",c,name);  
        snprintf(name,300,"%s",avcodec_get_name(avctx->codec_id));
        ADM_info("\t  Evaluating codec %d, %s\n",avctx->codec_id,name);
        if(c!=AV_PIX_FMT_VAAPI) continue;
#define FMT_V_CHECK(x,y) case AV_CODEC_ID_##x: outPix = AV_PIX_FMT_VAAPI; id = avctx->codec_id; break;
        switch(avctx->codec_id)  //AV_CODEC_ID_H265
        {
            FMT_V_CHECK(H264,H264)
            FMT_V_CHECK(H265,H265)
            FMT_V_CHECK(MPEG1VIDEO,MPEG1)
            FMT_V_CHECK(MPEG2VIDEO,MPEG2)
            FMT_V_CHECK(WMV3,WMV3)
            FMT_V_CHECK(VC1,VC1)
            FMT_V_CHECK(VP9,VP9)
            default: 
                ADM_info("No hw support for format %d\n",avctx->codec_id);
                continue;
                break;
        }
        break;
    }
    if(id==AV_CODEC_ID_NONE)
    {
        return AV_PIX_FMT_NONE;
    }
    // Check that the profile is supported
    VAProfile profile = VAProfileNone;
    switch(avctx->codec_id)
    {
        case AV_CODEC_ID_MPEG2VIDEO: profile = VAProfileMPEG2Main; break;
        case AV_CODEC_ID_H264: profile = VAProfileH264High; break;
#ifdef LIBVA_HEVC_DEC
        case AV_CODEC_ID_H265:
            switch(avctx->pix_fmt)
            {
                case AV_PIX_FMT_YUV420P:
                    profile = VAProfileHEVCMain;
                    break;
                case AV_PIX_FMT_YUV420P10LE:
                    ADM_info("10 bits H265\n");
                    profile = VAProfileHEVCMain10;
                    break;
                default:
                    ADM_warning("FF/LibVa: unknown pixel format %d\n",(int)avctx->pix_fmt);
                    return AV_PIX_FMT_NONE;
               break;
            }
            break;
#endif
        case AV_CODEC_ID_VC1: profile = VAProfileVC1Advanced; break;
#ifdef LIBVA_VP9_DEC
        case AV_CODEC_ID_VP9: profile = VAProfileVP9Profile0; break;
#endif
        default:
            ADM_info("Unknown codec (libVA)\n");
            return AV_PIX_FMT_NONE;
    }
    if(!admLibVA::supported(profile, avctx->coded_width, avctx->coded_height))
    {
        ADM_warning("Not supported by libVA\n");
        return AV_PIX_FMT_NONE;
    }
    if(avctx->hw_frames_ctx)
    {
        ADM_info("Re-using existing hw frames context.\n");
        return AV_PIX_FMT_VAAPI;
    }
    // Finish intialization of LIBVA decoder
    AVBufferRef *devRef = av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_VAAPI);
    if(!devRef)
    {
        ADM_warning("Cannot allocate hw device context\n");
        return AV_PIX_FMT_NONE;
    }

    AVHWDeviceContext *dc = (AVHWDeviceContext *)devRef->data;
    AVVAAPIDeviceContext *vaCtx = (AVVAAPIDeviceContext *)dc->hwctx;
    vaCtx->display = admLibVA::getVADisplay();
    if(av_hwdevice_ctx_init(devRef) < 0)
    {
        ADM_warning("Cannot init VA-API device context\n");
        av_buffer_unref(&devRef);
        return AV_PIX_FMT_NONE;
    }

    AVBufferRef *frameRef = av_hwframe_ctx_alloc(devRef);
    if(!frameRef)
    {
        ADM_warning("Cannot allocate hw frame context\n");
        av_buffer_unref(&devRef);
        return AV_PIX_FMT_NONE;
    }
    AVHWFramesContext *frameCtx = (AVHWFramesContext *)frameRef->data;
    frameCtx->format = AV_PIX_FMT_VAAPI;
    frameCtx->sw_format = (avctx->codec_id == AV_CODEC_ID_VP9)? AV_PIX_FMT_NV12 : AV_PIX_FMT_YUV420P;
    frameCtx->width = avctx->width;
    frameCtx->height = avctx->height;

    if(av_hwframe_ctx_init(frameRef) < 0)
    {
        ADM_warning("Cannot init VA-API frame context\n");
        av_buffer_unref(&frameRef);
        av_buffer_unref(&devRef);
        return AV_PIX_FMT_NONE;
    }

    avctx->hw_frames_ctx = frameRef;
    return AV_PIX_FMT_VAAPI;
}
}

/**
 * 
 * @param w
 * @param h
 * @param fcc
 * @param extraDataLen
 * @param extraData
 * @param bpp
 */
decoderFFLIBVA::decoderFFLIBVA(AVCodecContext *avctx,decoderFF *parent)
: ADM_acceleratedDecoderFF(avctx,parent)
{
    alive=false;
    // hw_frames_ctx should have been set up by get_format()
    if(!_context->hw_frames_ctx)
    {
        ADM_warning("hw_frames_ctx is NULL, cannot use lavc VA-API hw decoder.\n");
        return;
    }

    _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;

    for(int i=0;i<ADM_DEFAULT_SURFACE;i++)
        initSurfaceID[i]=VA_INVALID;

    for(int i=0;i<ADM_DEFAULT_SURFACE;i++)
    {
        ADM_vaSurface *admSurface=allocateADMVaSurface(avctx);
        if(!admSurface)
        {
            ADM_warning("Cannot allocate dummy surface\n");
            return;
        }
        aprintf("Created initial pool, %d : %x\n",i,admSurface->surface);
        initSurfaceID[i]=admSurface->surface;
        vaPool.allSurfaceQueue.append(admSurface);
        vaPool.freeSurfaceQueue.append(admSurface);
    }
    alive=true;
    _context->get_buffer2     = ADM_LIBVAgetBuffer;
    _context->draw_horiz_band = NULL;
    ADM_info("Successfully setup LIBVA hw accel\n");             
}

/**
 * \fn dtor
 */
decoderFFLIBVA::~decoderFFLIBVA()
{
    imageMutex.lock();
    int m=vaPool.allSurfaceQueue.size();
    int n=vaPool.freeSurfaceQueue.size();
    if(n!=m)
    {
        ADM_warning("Some surfaces are not reclaimed! (%d/%d)\n",n,m);
    }
    for(int i=0;i<n;i++)
    {
        delete vaPool.freeSurfaceQueue[i];
    }
    vaPool.freeSurfaceQueue.clear();
    imageMutex.unlock();
}
/**
 * \fn uncompress
 * \brief 
 * @param in
 * @param out
 * @return 
 */
bool decoderFFLIBVA::uncompress (ADMCompressedImage * in, ADMImage * out)
{
      
    aprintf("==> uncompress %s\n",_context->codec->long_name);
    if(out->refType==ADM_HW_LIBVA)
    {
            ADM_vaSurface *img=(ADM_vaSurface *)out->refDescriptor.refHwImage;
            markSurfaceUnused(img);
            out->refType=ADM_HW_NONE;
    }

    if(!_parent->getDrainingState() && !in->dataLength) // Null frame, silently skipped
    {
        out->_noPicture = 1;
        out->Pts=ADM_COMPRESSED_NO_PTS;
        out->refType=ADM_HW_NONE;
        ADM_info("[LibVa] Nothing to decode -> no Picture\n");
        return false;
    }

   // Put a safe value....
    out->Pts=in->demuxerPts;
    _context->reordered_opaque=in->demuxerPts;

    AVFrame *frame=_parent->getFramePointer();
    ADM_assert(frame);

    int ret = 0;

    if(_parent->getDrainingState())
    {
        if(_parent->getDrainingInitiated()==false)
        {
            avcodec_send_packet(_context, NULL);
            _parent->setDrainingInitiated(true);
        }
    }else if(!handover)
    {
        AVPacket *pkt = _parent->getPacketPointer();
        ADM_assert(pkt);
        pkt->data = in->data;
        pkt->size = in->dataLength;

        if(in->flags&AVI_KEY_FRAME)
            pkt->flags = AV_PKT_FLAG_KEY;
        else
            pkt->flags = 0;

        ret = avcodec_send_packet(_context, pkt);
        if(ret)
        {
            char er[AV_ERROR_MAX_STRING_SIZE]={0};
            av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, ret);
            ADM_warning("Ignoring error %d submitting packet to decoder (\"%s\")\n",ret,er);
        }
        av_packet_unref(pkt);
    }else
    {
        handover=false;
    }

    ret = avcodec_receive_frame(_context, frame);

    if(!_parent->decodeErrorHandler(ret))
        return false;

    if(frame->pict_type==AV_PICTURE_TYPE_NONE)
    {
        out->_noPicture=true;
        out->refType=ADM_HW_NONE;
        out->Pts= (uint64_t)(frame->reordered_opaque);
        ADM_info("[LIBVA] No picture \n");
        return false;
    }
    return readBackBuffer(frame,in,out);  
}
/**
 * 
 * @param decodedFrame
 * @param in
 * @param out
 * @return 
 */
bool     decoderFFLIBVA::readBackBuffer(AVFrame *decodedFrame, ADMCompressedImage * in, ADMImage * out)
{
    uint64_t pts_opaque=(uint64_t)(decodedFrame->reordered_opaque);
    out->Pts= (uint64_t)(pts_opaque);        
    out->flags=admFrameTypeFromLav(decodedFrame);
    out->_range=(decodedFrame->color_range==AVCOL_RANGE_JPEG)? ADM_COL_RANGE_JPEG : ADM_COL_RANGE_MPEG;
    out->refType=ADM_HW_LIBVA;
    out->refDescriptor.refCodec=this;
    ADM_vaSurface *img=(ADM_vaSurface *)(decodedFrame->data[0]);
    out->refDescriptor.refHwImage=img; // the ADM_vaImage in disguise
    markSurfaceUsed(img); // one ref for us too, it will be free when the image is cycled
    aprintf("ReadBack: Got image=%x surfaceId=%x\n",(int)(uintptr_t)decodedFrame->data[0],(int)img->surface);
    out->refDescriptor.refMarkUsed=libvaMarkSurfaceUsed;
    out->refDescriptor.refMarkUnused=libvaMarkSurfaceUnused;
    out->refDescriptor.refDownload=libvaRefDownload;
    return true;
}

//---

class ADM_hwAccelEntryLibVA : public ADM_hwAccelEntry
{
public: 
                            ADM_hwAccelEntryLibVA();
     virtual bool           canSupportThis(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt,enum AVPixelFormat &outputFormat);
     virtual                ADM_acceleratedDecoderFF *spawn( struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt );     
     virtual                ~ADM_hwAccelEntryLibVA() {};
};
/**
 * 
 */
ADM_hwAccelEntryLibVA::ADM_hwAccelEntryLibVA()
{
    name="VAAPI";
}
/**
 * 
 * @param avctx
 * @param fmt
 * @param outputFormat
 * @return 
 */
bool           ADM_hwAccelEntryLibVA::canSupportThis(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt,enum AVPixelFormat &outputFormat)
{
    bool enabled=false;
    prefs->get(FEATURES_LIBVA,&enabled);
    if(!enabled)
    {
        ADM_info("LibVA not enabled\n");
        return false;
    }
    enum AVPixelFormat ofmt=ADM_LIBVA_getFormat(avctx,fmt);
    if(ofmt==AV_PIX_FMT_NONE)
        return false;
    outputFormat=ofmt;
    return true;
}


/**
 * 
 * @param avctx
 * @param fmt
 * @return 
 */
ADM_acceleratedDecoderFF *ADM_hwAccelEntryLibVA::spawn( struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt )
{
    decoderFF *ff=(decoderFF *)avctx->opaque;
    decoderFFLIBVA *instance = new decoderFFLIBVA(avctx,ff);
    if(!instance->isAlive())
    {
        ADM_warning("VA-API HW accel. decoder could not be initialized, destroying it.\n");
        delete instance;
        instance=NULL;
    }
    return instance;
}
static ADM_hwAccelEntryLibVA libvaEntry;
/**
 * 
 * @return 
 */
bool initLIBVADecoder(void)
{
    ADM_info("Registering LIBVA hw decoder\n");
    ADM_hwAccelManager::registerDecoder(&libvaEntry);
    return true;
}

// EOF
