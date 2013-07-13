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
static void ADM_LIBVADraw(struct AVCodecContext *s,    const AVFrame *src, int offset[4],    int y, int type, int height);


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
    ADM_vaImage    *img=(ADM_vaImage *)v;
    decoderFFLIBVA *decoder=(decoderFFLIBVA *)cookie;
    imageMutex.lock();
    img->refCount++;
    imageMutex.unlock();
    
}
/**
    \fn markSurfaceUnused
    \brief mark the surfave as unused by the caller. Can be called multiple time.
*/
static bool libvaMarkSurfaceUnused(void *v, void * cookie)
{
    ADM_vaImage    *img=(ADM_vaImage *)v;
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
bool    decoderFFLIBVA::reclaimImage(ADM_vaImage *img)
{
        freeImageQueue.append(img);
        return true;
}
/**
    \fn vdpauRefDownload
    \brief Convert a VDPAU image to a regular image
*/

static bool libvaRefDownload(ADMImage *image, void *instance, void *cookie)
{
    ADM_vaImage    *img=(ADM_vaImage *)instance;
    decoderFFLIBVA *decoder=(decoderFFLIBVA *)cookie;
    return        admLibVA::imageToAdmImage(img,image);   
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
    VASurfaceID sid[ADM_MAX_SURFACE];
    
    alive=false;
    va_context=NULL;
    scratch=new ADMImageRef(w,h);
    
    // Allocate 17 surfaces, enough for the moment
    nbSurface=17;
    for(int i=0;i<nbSurface;i++)
    {
        surfaces[i]=admLibVA::allocateSurface(w,h);
        if(!surfaces[i]==VA_INVALID)
        {
            nbSurface=i;
            return;
        }
        freeQueue.pushFront(surfaces[i]);
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
    
    _context->opaque          = this;
    _context->thread_count    = 1;
    _context->get_buffer      = ADM_LIBVAgetBuffer;
    _context->release_buffer  = ADM_LIBVAreleaseBuffer ;
   // _context->draw_horiz_band = ADM_LIBVADraw;
     _context->draw_horiz_band = NULL;
    _context->get_format      = ADM_LIBVA_getFormat;
    _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
    _context->pix_fmt         = AV_PIX_FMT_VAAPI_VLD;
    _context->hwaccel_context = va_context;
    nbSurface=0;
    
    uint8_t *extraCopy=NULL;
    if(extraDataLen)
    {
            extraCopy=(uint8_t *)alloca(extraDataLen+FF_INPUT_BUFFER_PADDING_SIZE);
            memset(extraCopy,0,extraDataLen+FF_INPUT_BUFFER_PADDING_SIZE);
            memcpy(extraCopy,extraData,extraDataLen);
            _context->extradata = (uint8_t *) extraCopy;
            _context->extradata_size  = (int) extraDataLen;
    } 
    
     WRAP_Open(CODEC_ID_H264);
    //

     
     
   
      b_age = ip_age[0] = ip_age[1] = 256*256*256*64;
     alive=true;
}
#if 0
/**
 * \fn waitForSync
 * @param surface
 * @return 
 */
bool decoderFFLIBVA::waitForSync(void *surface)
{

    int count=1000;
    bool ready;
    while(--count)
    {
        if(!admXvba::syncSurface(xvba,surface,&ready))
        {
            ADM_warning("Sync surface failed\n");
            return false;
        }
        if(ready) break;
        aprintf("Surface not ready, waiting...\n");
        ADM_usleep(1000);
    }
    if(count)
        return true;

    return false;
}
#endif
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
    for(int i=0;i<nbSurface;i++)
    {
                admLibVA::destroySurface(surfaces[i]);        
                surfaces[i]=VA_INVALID;
    }
        
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
 * \brief First call ffmpeg, the 2nd part of the decoding will happen in ::goOn
 * @param in
 * @param out
 * @return 
 */
bool decoderFFLIBVA::uncompress (ADMCompressedImage * in, ADMImage * out)
{
    // First let ffmpeg prepare datas...

    aprintf("[LIBVA]>-------------uncompress>\n");
   
    if(!decoderFF::uncompress (in, scratch))
    {
        aprintf("[LIBVA] No data from libavcodec\n");
        return 0;
    }
    uint64_t p=(uint64_t )scratch->GetReadPtr(PLANAR_Y);
    VASurfaceID id=(VASurfaceID)p;
    out->Pts=scratch->Pts;
    out->flags=scratch->flags;
    
    imageMutex.lock();
    ADM_vaImage *img;
    if(  freeImageQueue.empty())
    {
        aprintf("Allocating new image\n");
        img=new ADM_vaImage(this,_w,_h);
        img->image=admLibVA::allocateYV12Image(_w,_h);
        if(!img->image)
        {
            delete img;
            ADM_warning("Cannot allocate image (libVA)\n");
            imageMutex.unlock();
            return false;
        }
        allImageQueue.append(img);
    }else
    {
        aprintf("Taking image from queue\n");
        img=freeImageQueue[0];
        freeImageQueue.popFront();
    }        
    imageMutex.unlock();
    if(!admLibVA::surfaceToImage(id,img))    
    {
        imageMutex.lock();
        freeImageQueue.append(img);
        imageMutex.unlock();
        ADM_warning("[LIBVA] Surface to image failed\n");
        return false;
    }
    out->refType=ADM_HW_LIBVA;
    out->refDescriptor.refCookie=this;
    out->refDescriptor.refInstance=img;
    out->refDescriptor.refMarkUsed=libvaMarkSurfaceUsed;
    out->refDescriptor.refMarkUnused=libvaMarkSurfaceUnused;
    out->refDescriptor.refDownload=libvaRefDownload;
    //libvaMarkSurfaceUsed(img,this);
    
  
    
    aprintf("uncompress : Got surface =0x%x\n",id);
    return true;
    
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
    \fn draw
    \brief callback invoked by lavcodec when a pic is ready to be decoded
*/
void ADM_LIBVADraw(struct AVCodecContext *s,    const AVFrame *src, int offset[4],    int y, int type, int height)
{
    decoderFFLIBVA *dec=(decoderFFLIBVA *)s->opaque;
    dec->goOn((AVFrame *)src,type);
    return ;
}


/**
    \fn releaseBuffer
*/
void decoderFFLIBVA::releaseBuffer(AVCodecContext *avctx, AVFrame *pic)
{
  uint64_t p=(uint64_t )pic->data[0];
  VASurfaceID s=(VASurfaceID)p;
  decoderFFLIBVA *x=(decoderFFLIBVA *)avctx->opaque;
    
  
  aprintf("Release Buffer : 0x%llx\n",s);
  
  for(int i=0; i<4; i++)
  {
    pic->data[i]= NULL;
  }
  
  x->freeQueue.pushBack(s);
}
/**
    \fn getBuffer
    \brief returns a VDPAU render masquerading as a AVFrame
*/
int decoderFFLIBVA::getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
    
    decoderFFLIBVA *x=(decoderFFLIBVA *)avctx->opaque;
    if(x->freeQueue.empty())
    {
        ADM_warning("Surface Queue is empty !\n");
        ADM_warning("Surface Queue is empty !\n");
        return -1;
        
    }
    VASurfaceID s= x->freeQueue[0];
    x->freeQueue.popFront();
    aprintf("Alloc Buffer : 0x%llx\n",s);
    uint8_t *p=(uint8_t *)s;
    pic->data[0]=p;
    pic->data[1]=p;
    pic->data[2]=p;
    pic->data[3]=p;
    pic->linesize[0]=0;
    pic->linesize[1]=0;
    pic->linesize[2]=0;
    pic->linesize[3]=0;
    pic->type=FF_BUFFER_TYPE_USER;
//    render->state  =0;
//    render->state |= FF_LIBVA_STATE_USED_FOR_REFERENCE;
//    render->state &= ~FF_LIBVA_STATE_DECODED;
//    render->psf=0;
    pic->reordered_opaque= avctx->reordered_opaque;
   // pic->opaque= avctx->opaque;
    return 0;
}
/**
    \fn goOn
    \brief Callback from ffmpeg when a pic is ready to be decoded
*/
void decoderFFLIBVA::goOn(  AVFrame *d,int type)
{
    aprintf("[LIBVA] Go on\n");
    uint64_t p=(uint64_t )d->data[3];
    d->data[0]=d->data[3];
    VASurfaceID s=(VASurfaceID)p;
    aprintf("[LIBVA] Surface ID=0x%x\n",(int)s);
    decode_status=true;

    return;

}


#endif
// EOF
