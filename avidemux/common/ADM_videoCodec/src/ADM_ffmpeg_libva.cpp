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

#ifdef USE_LIBVA
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
#include "libavcodec/vaapi.h"
#include "libavcodec/internal.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_render/GUI_render.h"
#include "ADM_ffmpeg_libva_internal.h"
#include "prefs.h"
#include "ADM_coreVideoCodec/ADM_hwAccel/ADM_coreLibVA/include/ADM_coreLibVA.h"
#include "ADM_codecLibVA.h"
#include "ADM_threads.h"
#include "ADM_vidMisc.h"


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
#ifdef USE_LIBVA
    if( admCoreCodecSupports(ADM_CORE_CODEC_FEATURE_LIBVA)==false)
    {
        GUI_Error_HIG("Error","Core has been compiled without libva support, but the application has been compiled with it.\nInstallation mismatch");
        libvaWorking=false;
    }

    if(false==admLibVA::init(&xinfo)) return false;
    libvaWorking=true;
    return true;
#endif
    return false;
}

//    out->refDescriptor.refCookie=FFLIBVADecode;
//    out->refDescriptor.refInstance=ADM_vaIMage;
/**
    \fn markSurfaceUsed
    \brief mark the surfave as used. Can be called multiple time.
*/
static bool libvaMarkSurfaceUsed(void *v, void * cookie)
{
    ADM_vaSurface    *img=(ADM_vaSurface *)v;
    decoderFFLIBVA *decoder=(decoderFFLIBVA *)cookie;
    imageMutex.lock();
    img->refCount++;
    imageMutex.unlock();
    return true;
    
}
/**
    \fn markSurfaceUnused
    \brief mark the surfave as unused by the caller. Can be called multiple time.
*/
static bool libvaMarkSurfaceUnused(void *v, void * cookie)
{
    ADM_vaSurface    *img=(ADM_vaSurface *)v;
    decoderFFLIBVA *decoder=(decoderFFLIBVA *)cookie;
    imageMutex.lock();
    img->refCount--;
    aprintf("Ref count is now %d\n",img->refCount);
    if(!img->refCount)
    {
        decoder->reclaimImage(img);
    }
    imageMutex.unlock();
    
   return true;
}
/**
 * \fn reclaimImage
 * \brief must be called with mutex held
 * @param image
 * @param instance
 * @param cookie
 * @return 
 */
bool    decoderFFLIBVA::reclaimImage(ADM_vaSurface *img)
{
        freeSurfaceQueue.append(img);
        return true;
}
/**
    \fn vdpauRefDownload
    \brief Convert a VDPAU image to a regular image
*/

static bool libvaRefDownload(ADMImage *image, void *instance, void *cookie)
{
    ADM_vaSurface    *img=(ADM_vaSurface *)instance;
    decoderFFLIBVA *decoder=(decoderFFLIBVA *)cookie;
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
 * 
 * @param w
 * @param h
 * @param fcc
 * @param extraDataLen
 * @param extraData
 * @param bpp
 */
static enum AVPixelFormat ADM_LIBVA_getFormat( struct AVCodecContext * avctx , const AVPixelFormat * fmt)
{
   const PixelFormat * cur = fmt;
   while(*cur != AV_PIX_FMT_NONE)
   {
       if(*cur==AV_PIX_FMT_VAAPI_VLD) 
       {
           aprintf(">---------->Match\n");
           return AV_PIX_FMT_VAAPI_VLD;
       }
       cur++;
   }
   ADM_warning(">---------->No LIBVA colorspace\n");
   return AV_PIX_FMT_NONE;

    
}

// dummy
decoderFFLIBVA::decoderFFLIBVA(uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, 
        uint8_t *extraData,uint32_t bpp)
:decoderFF (w,h,fcc,extraDataLen,extraData,bpp)
{
    
    VASurfaceID sid[ADM_MAX_SURFACE+1];
    
    alive=false;
    va_context=NULL;
    scratch=new ADMImageRef(w,h);
    
    // Allocate 17 surfaces, enough for the moment
    nbSurface=ADM_MAX_SURFACE;
    for(int i=0;i<nbSurface;i++)
    {
        surfaces[i]=admLibVA::allocateSurface(w,h);
        if(!surfaces[i]==VA_INVALID)
        {
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
    
    libva=admLibVA::createDecoder(w,h,nbSurface,sid);
    if(VA_INVALID==libva)
    {
        ADM_warning("Cannot create libva decoder\n");
        return;
    }
    
    va_context=new vaapi_context;
    memset(va_context,0,sizeof(*va_context)); // dangerous...
    
    if(!admLibVA::fillContext(va_context))
    {
        ADM_warning("Cannot get va context initialized for libavcodec\n");
        return ;
    }
    va_context->context_id=libva;
#if 0    
    WRAP_Open_TemplateLibVAByName("h264_vaapi",CODEC_ID_H264);
#else
      WRAP_Open_Template(avcodec_find_decoder,CODEC_ID_H264,"CODEC_ID_H264",CODEC_ID_H264,{\
            _context->opaque          = this; \
            _context->thread_count    = 1; \
            _context->get_buffer      = ADM_LIBVAgetBuffer; \
            _context->release_buffer  = ADM_LIBVAreleaseBuffer ;    \
            _context->draw_horiz_band = NULL; \
            _context->get_format      = ADM_LIBVA_getFormat; \
            _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD; \
            _context->pix_fmt         = AV_PIX_FMT_VAAPI_VLD; \
            _context->hwaccel_context = va_context;})
      
#endif
    nbSurface=0;
   
      b_age = ip_age[0] = ip_age[1] = 256*256*256*64;
     alive=true;
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
    int m=this->allSurfaceQueue.size();
    int n=freeSurfaceQueue.size();
    if(n!=m)
    {
        ADM_warning("Some surfaces are not reclaimed! (%d/%d)\n",n,m);
    }
    for(int i=0;i<n;i++)
    {
        delete freeSurfaceQueue[i];
    }
    freeSurfaceQueue.clear();
    imageMutex.unlock();
    
    nbSurface=0;
    if(libva!=VA_INVALID)
        admLibVA::destroyDecoder(libva);
    libva=VA_INVALID;

    if(scratch) delete scratch;
    scratch=NULL;    
    
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
    // First let ffmpeg prepare datas...

    aprintf("[LIBVA]>-------------uncompress>\n");
    printf("Incoming Pts=%s\n",ADM_us2plain(in->demuxerPts));
    if(!decoderFF::uncompress (in, scratch))
    {
        aprintf("[LIBVA] No data from libavcodec\n");
        return false;
    }
    printf("Out PTs =%s\n",ADM_us2plain(out->Pts));
    uint64_t p=(uint64_t )scratch->GetReadPtr(PLANAR_Y);
    VASurfaceID id=(VASurfaceID)p;
    out->Pts=scratch->Pts;
    out->flags=scratch->flags;
    
    ADM_vaSurface *img=lookupBySurfaceId(id);
    
    out->refType=ADM_HW_LIBVA;
    out->refDescriptor.refCookie=this;
    out->refDescriptor.refInstance=img;
    out->refDescriptor.refMarkUsed=libvaMarkSurfaceUsed;
    out->refDescriptor.refMarkUnused=libvaMarkSurfaceUnused;
    out->refDescriptor.refDownload=libvaRefDownload;
    aprintf("uncompress : Got surface =0x%x\n",id);
    printf("Out PTs =%s\n",ADM_us2plain(out->Pts));
    printf("Out Dts =%s\n",ADM_us2plain(out->Pts));
    return true;
    
}
/**
 * \fn lookupBySurfaceId
 * @param 
 * @return 
 */
ADM_vaSurface *decoderFFLIBVA::lookupBySurfaceId(VASurfaceID id)
{
    imageMutex.lock();
    int n=allSurfaceQueue.size();
    for(int i=0;i<n;i++)
        if(allSurfaceQueue[i]->surface==id)
        {
            imageMutex.unlock();
            return allSurfaceQueue[i];
        }
    imageMutex.unlock();
    ADM_warning("Lookup a non existing surface\n");
    ADM_assert(0);
    return NULL;
    
}
    
    
    
/**
    \fn ADM_VDPAUgetBuffer
    \brief trampoline to get a VDPAU surface
*/
int ADM_LIBVAgetBuffer(AVCodecContext *avctx, AVFrame *pic)
{
    decoderFFLIBVA *dec=(decoderFFLIBVA *)avctx->opaque;
    return dec->getBuffer(avctx,pic);
}
/**
 * \fn ADM_XVBAreleaseBuffer
 * @param avctx
 * @param pic
 */
void ADM_LIBVAreleaseBuffer(struct AVCodecContext *avctx, AVFrame *pic)
{
   decoderFFLIBVA *dec=(decoderFFLIBVA *)avctx->opaque;
   return dec->releaseBuffer(avctx,pic);
}



/**
    \fn releaseBuffer
*/
void decoderFFLIBVA::releaseBuffer(AVCodecContext *avctx, AVFrame *pic)
{
  uint64_t p=(uint64_t )pic->data[0];
  VASurfaceID s=(VASurfaceID)p;
  decoderFFLIBVA *x=(decoderFFLIBVA *)avctx->opaque;
    
  ADM_vaSurface *i=lookupBySurfaceId(s);
  
  aprintf("Release Buffer : 0x%llx\n",s);
  
  for(int i=0; i<4; i++)
  {
    pic->data[i]= NULL;
  }
  
  libvaMarkSurfaceUnused(i,this);
  
}
/**
    \fn getBuffer
    \brief returns a VDPAU render masquerading as a AVFrame
*/
int decoderFFLIBVA::getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
    
    decoderFFLIBVA *x=(decoderFFLIBVA *)avctx->opaque;
    
    imageMutex.lock();
    ADM_assert(!freeSurfaceQueue.empty());
    ADM_vaSurface *s= x->freeSurfaceQueue[0];
    x->freeSurfaceQueue.popFront();
    imageMutex.unlock();
    libvaMarkSurfaceUsed(s,this);
    
    aprintf("Alloc Buffer : 0x%llx\n",s);
    uint8_t *p=(uint8_t *)s->surface;
    pic->data[0]=p;
    pic->data[1]=p;
    pic->data[2]=p;
    pic->data[3]=p;
    pic->linesize[0]=0;
    pic->linesize[1]=0;
    pic->linesize[2]=0;
    pic->linesize[3]=0;
    pic->type=FF_BUFFER_TYPE_USER;
    pic->reordered_opaque= avctx->reordered_opaque;
 // I dont really understand what it is used for ....
    if(pic->reference)
    {
        ip_age[0]=ip_age[1]+1;
        ip_age[1]=1;
        b_age++;
    }else
    {
        ip_age[0]++;
        ip_age[1]++;
        b_age=1;
    }    
    return 0;
}



#endif
// EOF
