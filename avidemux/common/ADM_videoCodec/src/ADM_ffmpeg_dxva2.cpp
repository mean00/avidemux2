/***************************************************************************
            \file              ADM_ffmpeg_dxva2.cpp  
            \brief Decoder using half ffmpeg/half dxva2
            \author mean (c) 2016
 
 Very similar to dxva2


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
#include "libavcodec/dxva2.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_render/GUI_render.h"
#include "ADM_ffmpeg_dxva2_internal.h"
#include "prefs.h"
#include "ADM_coreLibVA.h"
#include "../private_inc/ADM_codecDxva2.h"
#include "ADM_threads.h"
#include "ADM_vidMisc.h"
#include "prefs.h"


static bool         dxva2Working=true;
static int  ADM_DXVA2getBuffer(AVCodecContext *avctx, AVFrame *pic);
static void ADM_DXVA2releaseBuffer(struct AVCodecContext *avctx, AVFrame *pic);



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
#if 0
static  bool dxva2MarkSurfaceUsed(void *instance,void *cookie)
{
    decoderFFDXVA2 *inst=(decoderFFDXVA2 *)instance;
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
static  bool dxva2MarkSurfaceUnused(void *instance,void *cookie)
{
    decoderFFDXVA2 *inst=(decoderFFDXVA2 *)instance ;
    ADM_vaSurface *s=(ADM_vaSurface *)cookie ;
    inst->markSurfaceUnused(s);
    return true;
}

/**
    \fn vdpauRefDownload
    \brief Convert a VASurface image to a regular image
*/
static bool dxva2RefDownload(ADMImage *image, void *instance, void *cookie)
{
    decoderFFDXVA2 *inst=(decoderFFDXVA2 *)instance ;
    ADM_vaSurface *s=(ADM_vaSurface *) cookie;
    return        s->toAdmImage(image);
}
#endif
/**
    \fn dxva2Usable
    \brief Return true if  dxva2 can be used...
*/
bool dxva2Usable(void)
{    
    bool v=true;
    if(!dxva2Working) return false;
    if(!prefs->get(FEATURES_DXVA2,&v)) v=false;
    return v;
}
#if 0
static ADM_vaSurface *allocateADMVaSurface(AVCodecContext *ctx)
{
    int widthToUse = (ctx->coded_width+ 1)  & ~1;
    int heightToUse= (ctx->coded_height+3)  & ~3;
    VASurfaceID surface=admLibVA::allocateSurface(widthToUse,heightToUse);
    if(surface==VA_INVALID)
    {
            ADM_warning("Cannot allocate surface (%d x %d)\n",(int)widthToUse,(int)heightToUse);
            return NULL;
    }
    ADM_vaSurface *img=new ADM_vaSurface(widthToUse,heightToUse);
    img->surface=surface;
    return img;
}
#endif
/**
    \fn dxva2Probe
    \brief Try loading vaapi...
*/
bool dxva2Probe(void)
{
    GUI_WindowInfo xinfo;
    void *draw;
    draw=UI_getDrawWidget();
    UI_getWindowInfo(draw,&xinfo );
    if( admCoreCodecSupports(ADM_CORE_CODEC_FEATURE_DXVA2)==false)
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Error"),QT_TRANSLATE_NOOP("adm","Core has been compiled without DXVA2 support, but the application has been compiled with it.\nInstallation mismatch"));
        dxva2Working=false;
    }

    if(false==admDxva2::init(&xinfo)) return false;
    dxva2Working=true;
    return true;
}
#if 0
/**
    \fn markSurfaceUsed
    \brief mark the surfave as used. Can be called multiple time.
*/
bool decoderFFDXVA2::markSurfaceUsed(ADM_vaSurface *s)
{
    imageMutex.lock();
    s->refCount++;
    imageMutex.unlock();
    return true;
    
}
bool        decoderFFDXVA2::markSurfaceUnused(VASurfaceID id)
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
bool decoderFFDXVA2::markSurfaceUnused(ADM_vaSurface *img)
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
#endif 
/**
    \fn ADM_DXVA2getBuffer
    \brief trampoline to get a DXVA2 surface
*/
int ADM_LIBDXVA2getBuffer(AVCodecContext *avctx, AVFrame *pic,int flags)
{
#if 0
    decoderFF *ff=(decoderFF *)avctx->opaque;
    decoderFFDXVA2 *dec=(decoderFFDXVA2 *)ff->getHwDecoder();
    ADM_assert(dec);
    return dec->getBuffer(avctx,pic);
#endif
    return -1;
}

/**
 * \fn ADM_XVBAreleaseBuffer
 * @param avctx
 * @param pic
 */
void ADM_LIBDXVA2releaseBuffer(void *opaque, uint8_t *data)
{
#if 0
   decoderFFDXVA2 *dec=(decoderFFDXVA2 *)opaque;
   ADM_assert(dec);
   VASurfaceID   surface=*(VASurfaceID *)(data);
   dec->markSurfaceUnused(surface);
#endif
   return -1;
}

/**
 * 
 * @param avctx
 * @param pic
 * @return 
 */
int decoderFFDXVA2::getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
#if 0        
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
                                     ADM_DXVA2releaseBuffer, 
                                     (void *)this,
                                     AV_BUFFER_FLAG_READONLY);
    
    aprintf("Alloc Buffer : 0x%llx, surfaceid=%x\n",s,(int)s->surface);
    pic->data[0]=(uint8_t *)s;
    pic->data[3]=(uint8_t *)(uintptr_t)s->surface;
    pic->reordered_opaque= avctx->reordered_opaque;    
    return 0;
#endif
    return -1;
}

/**
    \fn dxva2Cleanup
*/
bool dxva2Cleanup(void)
{
  // return admLibVA::cleanup();
    return false;
}






extern "C"
{

static enum AVPixelFormat ADM_DXVA2_getFormat(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt)
{
    int i;
    ADM_info("[DXVA2]: GetFormat\n");
    return AV_PIX_FMT_NONE;
#if 0    
    AVCodecID id=AV_CODEC_ID_NONE;
    AVPixelFormat c;
    AVPixelFormat outPix;
    for(i=0;fmt[i]!=AV_PIX_FMT_NONE;i++)
    {
        c=fmt[i];
        char name[300]={0};
        av_get_pix_fmt_string(name,sizeof(name),c);
        ADM_info("[DXVA2]: Evaluating PIX_FMT %d,%s\n",c,name);  
        av_get_codec_tag_string(name,sizeof(name),avctx->codec_id);
        ADM_info("\t  Evaluating codec %d,%s\n",avctx->codec_id,name);  
        
        if(c!=AV_PIX_FMT_VAAPI_VLD) continue;
#define FMT_V_CHECK(x,y)      case AV_CODEC_ID_##x:   outPix=AV_PIX_FMT_VAAPI_VLD;id=avctx->codec_id;break;
        
        
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
    // Finish intialization of DXVA2 decoder
    const AVHWAccel *accel=ADM_acceleratedDecoderFF::parseHwAccel(outPix,id,AV_PIX_FMT_VAAPI_VLD);
    if(accel)
    {
        ADM_info("Found matching hw accelerator : %s\n",accel->name);
        ADM_info("Successfully setup hw accel\n");
        return AV_PIX_FMT_VAAPI_VLD;
    }
    return AV_PIX_FMT_NONE;
#endif    
}
}
#if 0
/**
 * 
 * @param w
 * @param h
 * @param fcc
 * @param extraDataLen
 * @param extraData
 * @param bpp
 */
decoderFFDXVA2::decoderFFDXVA2(AVCodecContext *avctx,decoderFF *parent)
: ADM_acceleratedDecoderFF(avctx,parent)
{
    
    
    alive=false;
    _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
    
    for(int i=0;i<ADM_DEFAULT_SURFACE;i++)
        initSurfaceID[i]=VA_INVALID;
    
    for(int i=0;i<ADM_DEFAULT_SURFACE;i++)
    {
        ADM_vaSurface *admSurface=allocateADMVaSurface(avctx);
        if(!admSurface)
        {
            ADM_warning("Cannot allocate dummy surface\n");
            alive=false;
            return ;
        }
        aprintf("Created initial pool, %d : %x\n",i,admSurface->surface);
        initSurfaceID[i]=admSurface->surface;
        vaPool.allSurfaceQueue.append(admSurface);
        vaPool.freeSurfaceQueue.append(admSurface);
    }
    
   
    // create decoder
    vaapi_context *va_context=new vaapi_context;
    memset(va_context,0,sizeof(*va_context)); // dangerous...
    
    VAProfile profile; 
    switch(avctx->codec_id)
    {
        default:
        case AV_CODEC_ID_H264:
                                profile=VAProfileH264High;
                                break;
#ifdef DXVA2_HEVC_DEC       
        case AV_CODEC_ID_H265:
                                profile=VAProfileHEVCMain; // TODO VAProfileHEVCMain10
                                break;
#endif                                
#ifdef DXVA2_VP9_DEC
        case AV_CODEC_ID_VP9:
                                profile=VAProfileVP9Profile3;
                                break;
#endif
        case AV_CODEC_ID_VC1:
                                profile=VAProfileVC1Advanced;
                                break;
                                
    } 
    
    va_context->context_id=admLibVA::createDecoder(profile,avctx->coded_width,avctx->coded_height,ADM_DEFAULT_SURFACE,initSurfaceID); // this is most likely wrong
    if(va_context->context_id==VA_INVALID)
    {
        ADM_warning("Cannot create decoder\n");
        delete va_context;
        va_context=NULL;
        alive=false;
        return;
    }
    
    
    if(!admLibVA::fillContext(profile,va_context))
    {
        ADM_warning("Cannot get va context initialized for libavcodec\n");
        alive=false;
        return ;
    }    
    
    _context->hwaccel_context=va_context;
    alive=true;
    _context->get_buffer2     = ADM_DXVA2getBuffer;
    _context->draw_horiz_band = NULL;
    ADM_info("Successfully setup DXVA2 hw accel\n");             
}

/**
 * \fn dtor
 */
decoderFFDXVA2::~decoderFFDXVA2()
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
bool decoderFFDXVA2::uncompress (ADMCompressedImage * in, ADMImage * out)
{
      
    aprintf("==> uncompress %s\n",_context->codec->long_name);
    if(out->refType==ADM_HW_DXVA2)
    {
            ADM_vaSurface *img=(ADM_vaSurface *)out->refDescriptor.refHwImage;
            markSurfaceUnused(img);
            out->refType=ADM_HW_NONE;
    }

    if (!in->dataLength )	// Null frame, silently skipped
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
    int got_picture;
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data=in->data;
    pkt.size=in->dataLength;
    if(in->flags&AVI_KEY_FRAME)
        pkt.flags=AV_PKT_FLAG_KEY;
    else
        pkt.flags=0;
    
    AVFrame *frame=_parent->getFramePointer();
    ADM_assert(frame);
    int ret = avcodec_decode_video2 (_context, frame, &got_picture, &pkt);
    
    if(ret<0)
    {
        char er[2048]={0};
        av_make_error_string(er, sizeof(er)-1, ret);
        ADM_warning("Error %d in lavcodec (%s)\n",ret,er);
        out->refType=ADM_HW_NONE;
        return false;
    }
    if(frame->pict_type==AV_PICTURE_TYPE_NONE)
    {
        out->_noPicture=true;
        out->refType=ADM_HW_NONE;
        out->Pts= (uint64_t)(frame->reordered_opaque);
        ADM_info("[DXVA2] No picture \n");
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
bool     decoderFFDXVA2::readBackBuffer(AVFrame *decodedFrame, ADMCompressedImage * in, ADMImage * out)
{
    uint64_t pts_opaque=(uint64_t)(decodedFrame->reordered_opaque);
    out->Pts= (uint64_t)(pts_opaque);        
    out->flags=admFrameTypeFromLav(decodedFrame);
    out->refType=ADM_HW_DXVA2;
    out->refDescriptor.refCodec=this;
    ADM_vaSurface *img=(ADM_vaSurface *)(decodedFrame->data[0]);
    out->refDescriptor.refHwImage=img; // the ADM_vaImage in disguise
    markSurfaceUsed(img); // one ref for us too, it will be free when the image is cycled
    aprintf("ReadBack: Got image=%x surfaceId=%x\n",(int)(uintptr_t)decodedFrame->data[0],(int)img->surface);
    out->refDescriptor.refMarkUsed=dxva2MarkSurfaceUsed;
    out->refDescriptor.refMarkUnused=dxva2MarkSurfaceUnused;
    out->refDescriptor.refDownload=dxva2RefDownload;
    return true;
}
#endif
//---

class ADM_hwAccelEntryDxva2 : public ADM_hwAccelEntry
{
public: 
                            ADM_hwAccelEntryDxva2();
     virtual bool           canSupportThis(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt,enum AVPixelFormat &outputFormat);
     virtual                ADM_acceleratedDecoderFF *spawn( struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt );     
     virtual                ~ADM_hwAccelEntryDxva2() {};
};
/**
 * 
 */
ADM_hwAccelEntryDxva2::ADM_hwAccelEntryDxva2()
{
    name="DXVA2";
}
/**
 * 
 * @param avctx
 * @param fmt
 * @param outputFormat
 * @return 
 */
bool           ADM_hwAccelEntryDxva2::canSupportThis(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt,enum AVPixelFormat &outputFormat)
{
    bool enabled=false;
    prefs->get(FEATURES_DXVA2,&enabled);
    if(!enabled)
    {
        ADM_info("DXVA2 not enabled\n");
        return false;
    }
    enum AVPixelFormat ofmt=ADM_DXVA2_getFormat(avctx,fmt);
    if(ofmt==AV_PIX_FMT_NONE)
        return false;
    return false;
#if 0    
    outputFormat=ofmt;
    ADM_info("This is maybe supported by DXVA2\n");
    VAProfile profile=VAProfileNone;
    switch(avctx->codec_id)
    {
       case AV_CODEC_ID_H264: profile= VAProfileH264High;break;
#ifdef DXVA2_HEVC_DEC       
       case AV_CODEC_ID_H265: profile= VAProfileHEVCMain;break;;
#endif       
       case AV_CODEC_ID_VC1: profile= VAProfileVC1Advanced;break;
#ifdef DXVA2_VP9_DEC
       case AV_CODEC_ID_VP9: profile= VAProfileVP9Profile3;break;
#endif
       default:
           return false;
    }
    if(!admLibVA::supported(profile))
    {
        ADM_warning("Not supported by libVA\n");
        return false;
    }
    return true;
#endif    
}


/**
 * 
 * @param avctx
 * @param fmt
 * @return 
 */
ADM_acceleratedDecoderFF *ADM_hwAccelEntryLibVA::spawn( struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt )
{
    //decoderFF *ff=(decoderFF *)avctx->opaque;
    //return new decoderFFDXVA2(avctx,ff);
    return null
}
static ADM_hwAccelEntryLibVA dxva2Entry;
/**
 * 
 * @return 
 */
bool initDXVA2Decoder(void)
{
    ADM_info("Registering DXVA2 hw decocer\n");
    ADM_hwAccelManager::registerDecoder(&dxva2Entry);
    return true;
}

// EOF
