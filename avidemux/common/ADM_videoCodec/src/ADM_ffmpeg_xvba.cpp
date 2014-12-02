/***************************************************************************
            \file              ADM_ffmpeg_xvba.cpp  
            \brief Decoder using half ffmpeg/half VDPAU

 Strongly derived from xbmc_xvba
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

#ifdef USE_XVBA
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
#include "libavcodec/xvba.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_render/GUI_render.h"
#include "ADM_ffmpeg_xvba_internal.h"
#include "prefs.h"
#include "ADM_coreVideoCodec/ADM_hwAccel/ADM_coreXvba/include/ADM_coreXvba.h"
#include "ADM_codecXvba.h"
#include "ADM_threads.h"
#include "ADM_vidMisc.h"


static bool         xvbaWorking=true;
static admMutex     surfaceMutex;
static bool         destroyingFlag=false;
static BVector   <void *> destroyedList;

#if 0
#define aprintf(...) {}
#else
#define aprintf ADM_info
#endif

typedef enum 
{
    ADM_XVBA_INVALID=0,
    ADM_XVBA_H264=1,
    ADM_XVBA_MPEG2=2,
    ADM_XVBA_VC1=3
}ADM_XVBA_TYPE;

// Trampoline
static void ADM_XVBADraw(struct AVCodecContext *s,    const AVFrame *src, int offset[4],    int y, int type, int height);
static void ADM_XVBAreleaseBuffer(struct AVCodecContext *avctx, AVFrame *pic);
static int  ADM_XVBAgetBuffer(AVCodecContext *avctx, AVFrame *pic);

/**
    \fn hasBeenDestroyed
    \brief If the context has been destroyed, dont attempt to manipulate the buffers attached
*/
static bool hasBeenDestroyed(void *ptr)
{
    int nb=destroyedList.size();
    for(int i=0;i<nb;i++)
    {
        if(destroyedList[i]==ptr)
            return true;
    }
    return false;
}

/**
    \fn markSurfaceUsed
    \brief mark the surfave as used. Can be called multiple time.
*/
static bool xvbaMarkSurfaceUsed(void *v, void * cookie)
{
  
    return true;
}
/**
    \fn markSurfaceUnused
    \brief mark the surfave as unused by the caller. Can be called multiple time.
*/
static bool xvbaMarkSurfaceUnused(void *v, void * cookie)
{
    return true;
}
/**
    \fn vdpauRefDownload
    \brief Convert a VDPAU image to a regular image
*/

static bool vdpauRefDownload(ADMImage *image, void *instance, void *cookie)
{
  
    return true;
}

/**
    \fn vdpauUsable
    \brief Return true if  vdpau can be used...
*/
bool xvbaUsable(void)
{    
    bool v=true;
    if(!xvbaWorking) return false;
    if(!prefs->get(FEATURES_XVBA,&v)) v=false;
    return v;
}
/**
    \fn vdpauProbe
    \brief Try loading vdpau...
*/
bool xvbaProbe(void)
{
    GUI_WindowInfo xinfo;
    void *draw;
    draw=UI_getDrawWidget();
    UI_getWindowInfo(draw,&xinfo );
#ifdef USE_XVBA
    if( admCoreCodecSupports(ADM_CORE_CODEC_FEATURE_XVBA)==false)
    {
        GUI_Error_HIG("Error","Core has been compiled without xvba support, but the application has been compiled with it.\nInstallation mismatch");
        xvbaWorking=false;
    }

    if(false==admXvba::init(&xinfo)) return false;
    xvbaWorking=true;
    return true;
#endif
    return false;
}
/**
    \fn vdpauCleanup
*/
bool xvbaCleanup(void)
{
   return admXvba::cleanup();
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
static enum AVPixelFormat ADM_XVBA_getFormat( struct AVCodecContext * avctx , const AVPixelFormat * fmt)
{
   const PixelFormat * cur = fmt;
   while(*cur != AV_PIX_FMT_NONE)
   {
       if(*cur==AV_PIX_FMT_XVBA_VLD) 
       {
           aprintf(">---------->Match\n");
           return AV_PIX_FMT_XVBA_VLD;
       }
       cur++;
   }
   ADM_warning(">---------->No XVBA colorspace\n");
   return AV_PIX_FMT_NONE;

    
}

// dummy
decoderFFXVBA::decoderFFXVBA(uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, 
        uint8_t *extraData,uint32_t bpp)
:decoderFF (w,h,fcc,extraDataLen,extraData,bpp)
{
    alive=false;
    scratch=new ADMImageRef(w,h);
    pictureDescriptor=dataBuffer=qmBuffer=ctrlBuffer[0]=NULL;
    ctrlBufferCount=0;
    tmpYV12Buffer=new uint8_t[w*h*2];
    xvba=admXvba::createDecoder(w,h);
    if(!xvba) return;
    
     pictureDescriptor=admXvba::createDecodeBuffer(xvba,XVBA_PICTURE_DESCRIPTION_BUFFER);
     dataBuffer=admXvba::createDecodeBuffer(xvba,XVBA_DATA_BUFFER);
     ctrlBuffer[0]=admXvba::createDecodeBuffer(xvba,XVBA_DATA_CTRL_BUFFER);
     ctrlBufferCount++;
     qmBuffer=admXvba::createDecodeBuffer(xvba,XVBA_QM_BUFFER);
#define CHECK_BUFFER(x)     if(!x) {ADM_warning("Failed to allocate "#x"\n");return;}
     CHECK_BUFFER(pictureDescriptor)
     CHECK_BUFFER(dataBuffer)
     CHECK_BUFFER(ctrlBuffer[0])
     CHECK_BUFFER(qmBuffer)

    
    
    _context->opaque          = this;
    _context->thread_count    = 1;
    _context->get_buffer      = ADM_XVBAgetBuffer;
    _context->release_buffer  = ADM_XVBAreleaseBuffer ;
    _context->draw_horiz_band = ADM_XVBADraw;
    _context->get_format      = ADM_XVBA_getFormat;
    _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
    _context->pix_fmt         = AV_PIX_FMT_XVBA_VLD;
    
    
    
    if(extraDataLen)
    {
            _context->extradata = (uint8_t *) _extraDataCopy;
            _context->extradata_size  = (int) extraDataLen;
    } 
    
     WRAP_Open(CODEC_ID_H264);
    //

     
     
   
      b_age = ip_age[0] = ip_age[1] = 256*256*256*64;
     alive=true;
}
/**
 * \fn waitForSync
 * @param surface
 * @return 
 */
bool decoderFFXVBA::waitForSync(void *surface)
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

/**
 * \fn dtor
 */
decoderFFXVBA::~decoderFFXVBA()
{
    if(_context) // duplicate ~decoderFF to make sure in transit buffers are 
                 // released
    {
        avcodec_close (_context);
        av_free(_context);
        _context=NULL;
    }
    if(xvba)
    {
        aprintf("Deleting surfaces \n");
        if(allQueue.size()!=this->freeQueue.size())
        {
            ADM_warning("Some surface are unaccounted for (%d / %d)\n",allQueue.size(),freeQueue.size());
        }
        while(allQueue.size())
        {
            xvba_render_state *r=allQueue[0];
            allQueue.popFront();
            waitForSync(r->surface);
            admXvba::destroySurface(xvba,r->surface);
            free(r);
        }
        // destroy buffers
        
        #define DEL_BUFFER(x)     aprintf("Deleting "#x"\n");if(x) {admXvba::destroyDecodeBuffer(xvba,x);}
         DEL_BUFFER(pictureDescriptor)
         DEL_BUFFER(dataBuffer)     
         DEL_BUFFER(qmBuffer)
         for(int i=0;i<ctrlBufferCount;i++)             
             admXvba::destroyDecodeBuffer(xvba,ctrlBuffer[i]);
        // delete decoder
        aprintf("Destroying session\n");
        admXvba::destroyDecoder(xvba);
        xvba=NULL;
    }
    if(scratch) delete scratch;
    scratch=NULL;            
}
/**
 * \fn uncompress
 * \brief First call ffmpeg, the 2nd part of the decoding will happen in ::goOn
 * @param in
 * @param out
 * @return 
 */
bool decoderFFXVBA::uncompress (ADMCompressedImage * in, ADMImage * out)
{
    // First let ffmpeg prepare datas...
    xvba_copy=out;
    decodedCount=0;
    decode_status=false;
    aprintf("[XVBA]>-------------uncompress>\n");
   
    if(!decoderFF::uncompress (in, scratch))
    {
        aprintf("[XVBA] No data from libavcodec\n");
        return 0;
    }
    if(decode_status!=true)
    {
        printf("[XVBA] error in renderDecode (bad decode status)\n");
        return false;
    }
    struct xvba_render_state *rndr = (struct xvba_render_state *)scratch->GetReadPtr(PLANAR_Y);
    
    if(!waitForSync(rndr->surface))
    {
            ADM_warning("Sync surface failed\n");
            return false;
    }
    aprintf("[XVBA] Surface ready :%x ...\n",rndr->surface);
    
    rndr->state |= FF_XVBA_STATE_DECODED;        
    if(!admXvba::transfer(xvba,_w,_h,rndr->surface,out,tmpYV12Buffer))
    {
        ADM_warning("Cannot transfer\n");
        return false;
    }
    
    out->Pts=scratch->Pts;
    out->flags=scratch->flags;
    aprintf("PTs = %s\n",ADM_us2plain(out->Pts));
    return true;
    
}
/**
    \fn ADM_VDPAUgetBuffer
    \brief trampoline to get a VDPAU surface
*/
int ADM_XVBAgetBuffer(AVCodecContext *avctx, AVFrame *pic)
{
    decoderFFXVBA *dec=(decoderFFXVBA *)avctx->opaque;
    return dec->getBuffer(avctx,pic);
}
/**
 * \fn ADM_XVBAreleaseBuffer
 * @param avctx
 * @param pic
 */
void ADM_XVBAreleaseBuffer(struct AVCodecContext *avctx, AVFrame *pic)
{
   decoderFFXVBA *dec=(decoderFFXVBA *)avctx->opaque;
   return dec->releaseBuffer(avctx,pic);
}


/**
    \fn draw
    \brief callback invoked by lavcodec when a pic is ready to be decoded
*/
void ADM_XVBADraw(struct AVCodecContext *s,    const AVFrame *src, int offset[4],    int y, int type, int height)
{
    decoderFFXVBA *dec=(decoderFFXVBA *)s->opaque;
    dec->goOn(src,type);
    return ;
}


/**
    \fn releaseBuffer
*/
void decoderFFXVBA::releaseBuffer(AVCodecContext *avctx, AVFrame *pic)
{
  xvba_render_state * render;
  int i;
  decoderFFXVBA *x=(decoderFFXVBA *)avctx->opaque;
  render=(xvba_render_state*)pic->data[0];
  
#if 0
  int n=x->allQueue.size();
  bool found=false;
  for(int i=0;i<n;i++)
  {
      if(render==x->allQueue[i])
      {
          found=true;
          break;
      }
  }    
  if(false==found)
  {
      ADM_warning("***** Freeing invalid buffer *******\n");
      exit(-1);
  }
#endif
  
  aprintf("Release Buffer : 0x%llx\n",render);
  ADM_assert(render);
  for(i=0; i<4; i++)
  {
    pic->data[i]= NULL;
  }
  render->state &=~ FF_XVBA_STATE_USED_FOR_REFERENCE;

  x->freeQueue.pushBack(render);
}
/**
    \fn getBuffer
    \brief returns a VDPAU render masquerading as a AVFrame
*/
int decoderFFXVBA::getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
    
    decoderFFXVBA *x=(decoderFFXVBA *)avctx->opaque;
    xvba_render_state * render;
    if(!x->freeQueue.size())
    {
        aprintf("[XVBA] Allocating NEW surface\n");
        void *surface=admXvba::allocateSurface(x->xvba,x->_w,x->_h);
        if(!surface)
        {
            ADM_warning("[XVBA]Cannot allocate surface\n");
            return -1;
        }
        ADM_info("Allocated surface %llx\n",surface);
        render = (xvba_render_state*)calloc(sizeof(xvba_render_state), 1);
        render->surface=surface;
        render->buffers_alllocated=0;
        render->iq_matrix=(XVBAQuantMatrixAvc*)this->qmBuffer->bufferXVBA;
        render->picture_descriptor=(XVBAPictureDescriptor*)this->pictureDescriptor->bufferXVBA;
        x->allQueue.append(render);

    }else
    {
        // Get an image       
        render=x->freeQueue[0];
        x->freeQueue.popFront();
    }
    aprintf("Alloc Buffer : 0x%llx\n",render);
    pic->data[0]=(uint8_t *)render;
    pic->data[1]=(uint8_t *)render;
    pic->data[2]=(uint8_t *)render;
    pic->linesize[0]=0;
    pic->linesize[1]=0;
    pic->linesize[2]=0;
    pic->type=FF_BUFFER_TYPE_USER;
    render->state  =0;
    render->state |= FF_XVBA_STATE_USED_FOR_REFERENCE;
    render->state &= ~FF_XVBA_STATE_DECODED;
    render->psf=0;
    pic->reordered_opaque= avctx->reordered_opaque;
    pic->opaque= avctx->opaque;
    return 0;
}
/**
    \fn goOn
    \brief Callback from ffmpeg when a pic is ready to be decoded
*/
void decoderFFXVBA::goOn( const AVFrame *d,int type)
{
   
   struct xvba_render_state *rndr = (struct xvba_render_state *)d->data[0]; 
   aprintf("[XVBA]Decode Buffer : 0x%llx\n",rndr);
   aprintf("[XVBA]Surface  : 0x%llx\n",rndr->surface);
   if(!rndr)
   {
       ADM_warning("Bad context\n");
       return;
   }
   if(decodedCount)
   {
       ADM_warning("Multilple call to goOn\n");
       exit(-1);
   }
   decodedCount++;
   aprintf("-- decode start --\n");
   if(!admXvba::decodeStart(xvba,rndr->surface))
   {
       ADM_warning("Decode start failed\n");
       return;
   }
   if(!admXvba::decode1(xvba,pictureDescriptor,qmBuffer))
    {
        ADM_warning("Decode failed\n");
        return;
    } 
   // Make sure we have enough slices
   if(rndr->num_slices>this->ctrlBufferCount)
   {
       aprintf("Increasing ctrl buffer from %d to %d\n",ctrlBufferCount,rndr->num_slices);
       for(int j=ctrlBufferCount;j<rndr->num_slices;j++)
       {
           ctrlBuffer[j]=admXvba::createDecodeBuffer(xvba,XVBA_DATA_CTRL_BUFFER);
       }
       ctrlBufferCount=rndr->num_slices;
   }
 //-----------
  const    uint8_t startCode[] = {0x00,0x00,0x01};
  const int header=3;
  int offset = 0;  
  
  dataBuffer->data_size_in_buffer = 0;
  uint8_t *dest=(uint8_t *)dataBuffer->bufferXVBA;
  
   // check for potential buffer overflow
  int available= dataBuffer->buffer_size;
  int toCopy=0;   
  for(int i=0;i< rndr->num_slices; i++)
  {
      toCopy+=rndr->buffers[i].size+header;
  }
  if(toCopy>available)
  {
      ADM_warning("[XVBA] Too much data to copy, not enough space in buffer\n");
      return;
  }
  
  for (unsigned int j = 0; j < rndr->num_slices; ++j)
  {
    unsigned int bytesToCopy = rndr->buffers[j].size;
    uint8_t      *data=(uint8_t *)rndr->buffers[j].buffer;
    memcpy(dest+offset,  startCode, header);
    memcpy(dest+offset+header, data,  bytesToCopy);
    
    XVBADataCtrl *dataControl;
    dataControl = (XVBADataCtrl *)ctrlBuffer[j]->bufferXVBA;
    dataControl->SliceDataLocation = offset;
    dataControl->SliceBytesInBuffer = bytesToCopy+header;
    dataControl->SliceBitsInBuffer = dataControl->SliceBytesInBuffer * 8;
    dataBuffer->data_size_in_buffer += dataControl->SliceBytesInBuffer;
    offset += dataControl->SliceBytesInBuffer;
  }

  int bufSize = dataBuffer->data_size_in_buffer;
  int padding = bufSize % 128;
  if (padding)
  {
    padding = 128 - padding;
    dataBuffer->data_size_in_buffer += padding;
    memset((uint8_t *)dest+bufSize,0,padding);
  }

  for (unsigned int i = 0; i < rndr->num_slices; ++i)
  {    
    aprintf("-- decode 2 --\n");
    if(!admXvba::decode2(xvba,dataBuffer,ctrlBuffer[i]))
    {
        ADM_warning("Decode failed\n");
        return;
    } 
  }
  
  //-------------- 
   aprintf("-- decode end --\n");

   if(!admXvba::decodeEnd(xvba))
   {
       ADM_warning("DecodeEnd failed\n");
       return;
   } 
   decode_status=true;
   aprintf("[XVBA] End goOn\n");
   //
    return;
}
#endif
// EOF
