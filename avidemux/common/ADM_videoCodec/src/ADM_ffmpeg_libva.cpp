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
#include "libavcodec/vaapi.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_render/GUI_render.h"
#include "ADM_ffmpeg_libva_internal.h"
#include "prefs.h"
#include "ADM_coreVideoCodec/ADM_hwAccel/ADM_coreLibVA/include/ADM_coreLibVA.h"
#include "../private_inc/ADM_codecLibVA.h"
#include "ADM_threads.h"
#include "ADM_vidMisc.h"
#include "prefs.h"


static bool         libvaWorking=true;
static admMutex     imageMutex;
static int  ADM_LIBVAgetBuffer(AVCodecContext *avctx, AVFrame *pic);
static void ADM_LIBVAreleaseBuffer(struct AVCodecContext *avctx, AVFrame *pic);



#if 1
#define aprintf(...) {}
#else
#define aprintf ADM_info
#endif




/**
    \fn libvaUsable
    \brief Return true if  libva can be used...
*/
bool libvaUsable(void)
{    
    bool v=true;
    if(!libvaWorking) return false;
    if(!prefs->get(FEATURES_LIBVA,&v)) v=false;
    return v;
}
/**
    \fn libvaProbe
    \brief Try loading vaapi...
*/
bool libvaProbe(void)
{
    GUI_WindowInfo xinfo;
    void *draw;
    draw=UI_getDrawWidget();
    UI_getWindowInfo(draw,&xinfo );
    if( admCoreCodecSupports(ADM_CORE_CODEC_FEATURE_LIBVA)==false)
    {
        GUI_Error_HIG("Error","Core has been compiled without LIBVA support, but the application has been compiled with it.\nInstallation mismatch");
        libvaWorking=false;
    }

    if(false==admLibVA::init(&xinfo)) return false;
    libvaWorking=true;
    return true;
}

//    out->refDescriptor.refCookie=FFLIBVADecode;
//    out->refDescriptor.refInstance=ADM_vaIMage;
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
   aprintf("Ref count is now %d\n",img->refCount);
   if(!img->refCount)
   {
        vaPool.freeSurfaceQueue.append(img);
   }
   imageMutex.unlock();
   return true;
}
 
/**
    \fn ADM_VDPAUgetBuffer
    \brief trampoline to get a VDPAU surface
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
   VASurfaceID   surface=(VASurfaceID )(uintptr_t)data;
   dec->markSurfaceUnused(surface);
}

/**
    \fn getBuffer
    \brief returns a VDPAU render masquerading as a AVFrame
*/
int decoderFFLIBVA::getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
        
    imageMutex.lock();
    if(vaPool.freeSurfaceQueue.empty())
    {
         int widthToUse = (avctx->coded_width+ 1)  & ~1;
         int heightToUse= (avctx->coded_height+3)  & ~3;
            VASurfaceID surface=admLibVA::allocateSurface(widthToUse,heightToUse);
            if(surface==VA_INVALID)
            {
                ADM_warning("Cannot allocate surface (%d x %d)\n",(int)widthToUse,(int)heightToUse);
                imageMutex.unlock();
                return -1;
            }
            ADM_vaSurface *img=new ADM_vaSurface(widthToUse,heightToUse);
            img->surface=surface;
            vaPool.freeSurfaceQueue.append(img);
            vaPool.allSurfaceQueue.append(img);
    }
    ADM_vaSurface *s= vaPool.freeSurfaceQueue[0];
    vaPool.freeSurfaceQueue.popFront();
    imageMutex.unlock();
    s->refCount=0;
    markSurfaceUsed(s);
    aprintf("Alloc Buffer : 0x%llx\n",s);
    pic->data[0]=(uint8_t *)s;
    pic->data[3]=(uint8_t *)(uintptr_t)s->surface;
    pic->reordered_opaque= avctx->reordered_opaque;    
    // It only works because surface is the 1st field of render!
    pic->buf[0]=av_buffer_create((uint8_t *)&(s->surface),  // Maybe a memleak here...
                                     sizeof(s->surface),
                                     ADM_LIBVAreleaseBuffer, 
                                     (void *)this,
                                     AV_BUFFER_FLAG_READONLY);
    return 0;
}
/**
    \fn vdpauRefDownload
    \brief Convert a VDPAU image to a regular image
*/

static bool libvaRefDownload(ADMImage *image, ADM_vaSurface *img, decoderFFLIBVA *decoder)
{
    return        img->toAdmImage(image);
}

/**
    \fn vdpauCleanup
*/
bool libvaCleanup(void)
{
   return admLibVA::cleanup();
}


/**
 * \fn lookupBySurfaceId
 * @param 
 * @return 
 */
ADM_vaSurface *decoderFFLIBVA::lookupBySurfaceId(VASurfaceID id)
{
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
  


/**
 * 
 * @param w
 * @param h
 * @param fcc
 * @param extraDataLen
 * @param extraData
 * @param bpp
 */
static const AVHWAccel *parseHwAccel(enum AVPixelFormat pix_fmt,AVCodecID id)
{
    AVHWAccel *hw=av_hwaccel_next(NULL);
    
    while(hw)
    {
        ADM_info("Trying %s, %d : %d, codec =%d : %d\n",hw->name,hw->pix_fmt,pix_fmt,hw->id,id);
        if (hw->pix_fmt == AV_PIX_FMT_VAAPI_VLD && id==hw->id)
            return hw;
        hw=av_hwaccel_next(hw);
    }
    return NULL;
}

extern "C"
{

static enum AVPixelFormat ADM_LIBVA_getFormat(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt)
{
    int i;
    ADM_info("[vdpau]: GetFormat\n");
    AVCodecID id=AV_CODEC_ID_NONE;
    AVPixelFormat c;
    AVPixelFormat outPix;
    for(i=0;fmt[i]!=AV_PIX_FMT_NONE;i++)
    {
        c=fmt[i];
        ADM_info("[vdpau]: Evaluating %d\n",c);
        if(c!=AV_PIX_FMT_VAAPI_VLD) continue;
#define FMT_V_CHECK(x,y)      case AV_CODEC_ID_##x:   outPix=AV_PIX_FMT_VAAPI_VLD;id=avctx->codec_id;break;
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
        return AV_PIX_FMT_VAAPI_VLD;
    }
    return AV_PIX_FMT_NONE;
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
    
    VASurfaceID sid[ADM_MAX_SURFACE+1];
    alive=false;
    va_context=NULL;    
    
#if 0    
    // Allocate 17 surfaces, enough for the moment
    
    nbSurface=ADM_MAX_SURFACE;
    for(int i=0;i<nbSurface;i++)
    {

        surfaces[i]=admLibVA::allocateSurface(w,h);
        if(surfaces[i]==VA_INVALID)
        {
            ADM_warning("Cannot allocate surface (%d x %d)\n",(int)w,(int)h);
            nbSurface=i;
            return;
        }
        ADM_vaSurface *img=new ADM_vaSurface(this,w,h);
        img->surface=surfaces[i];
        freeSurfaceQueue.append(img);
        allSurfaceQueue.append(img);
        sid[i]=surfaces[i];
    }
    ADM_info("Preallocated %d surfaces\n",nbSurface);
    // Linearize surface
#endif        
    va_context=new vaapi_context;
    memset(va_context,0,sizeof(*va_context)); // dangerous...
    _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
    
    
#if 0
      
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
        if(!admLibVA::fillContext(va_context))
    {
        ADM_warning("Cannot get va context initialized for libavcodec\n");
        return ;
    }
#endif
    
    va_context->context_id=libva;  
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
    if(_context) // duplicate ~decoderFF to make sure in transit buffers are 
                 // released
    {
        avcodec_close (_context);
        av_free(_context);
        _context=NULL;
    }
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
    
    nbSurface=0;
    if(libva!=VA_INVALID)
        admLibVA::destroyDecoder(libva);
    libva=VA_INVALID;
 
    
    if(va_context)
    {
        delete va_context;
        va_context=NULL;
    }
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
            ADM_vaSurface *img=(ADM_vaSurface *)out->refDescriptor.refCookie;
            markSurfaceUnused(img);
            out->refType=ADM_HW_NONE;
    }

    if (!in->dataLength )	// Null frame, silently skipped
    {
        out->_noPicture = 1;
        out->Pts=ADM_COMPRESSED_NO_PTS;
        ADM_info("[LibVa] Nothing to decode -> no Picture\n");
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
    
    AVFrame *frame=_parent->getFramePointer();
    ADM_assert(frame);
    int ret = avcodec_decode_video2 (_context, frame, &got_picture, &pkt);
    
    if(ret<0)
    {
        char er[2048]={0};
        av_make_error_string(er, sizeof(er)-1, ret);
        ADM_warning("Error %d in lavcodec (%s)\n",ret,er);
        return false;
    }
    if(frame->pict_type==AV_PICTURE_TYPE_NONE)
    {
        out->_noPicture=true;
        out->Pts= (uint64_t)(frame->reordered_opaque);
        ADM_info("[VDPAU] No picture \n");
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
#if 0
   // VdpSurface *surface=decodedFrame->data[3];
    struct vdpau_render_state *rndr = (struct vdpau_render_state *)decodedFrame->data[0];
    ADM_assert(rndr);
    aprintf("Decoding ===> Got surface = %d\n",rndr->surface);
    out->refType=ADM_HW_VDPAU;
    out->refDescriptor.refCookie=(void *)rndr;
    out->refDescriptor.refInstance  =&vdpau;
    out->refDescriptor.refMarkUsed  =libvaMarkSurfaceUsed;
    out->refDescriptor.refMarkUnused=libvaMarkSurfaceUnused;
    out->refDescriptor.refDownload  =vdpauRefDownload;
    vdpauMarkSurfaceUsed(&vdpau,(void *)rndr);
#endif    
    uint64_t pts_opaque=(uint64_t)(decodedFrame->reordered_opaque);
    out->Pts= (uint64_t)(pts_opaque);    
    out->flags=admFrameTypeFromLav(decodedFrame);
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
    ADM_info("This is supported by LIBVA\n");
    return true;
}
ADM_acceleratedDecoderFF *ADM_hwAccelEntryLibVA::spawn( struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt )
{
    decoderFF *ff=(decoderFF *)avctx->opaque;
    return new decoderFFLIBVA(avctx,ff);
}
static ADM_hwAccelEntryLibVA libvaEntry;
/**
 * 
 * @return 
 */
bool initLIBVADecoder(void)
{
    ADM_info("Registering LIBVA hw decocer\n");
    ADM_hwAccelManager::registerDecoder(&libvaEntry);
    return true;
}

// EOF
