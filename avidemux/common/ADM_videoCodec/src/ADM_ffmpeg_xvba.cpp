/***************************************************************************
            \file              ADM_ffmpeg_xvba.cpp  
            \brief Decoder using half ffmpeg/half VDPAU

 Derived from xbmc_xvba
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
#include "libavcodec/xvba.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_render/GUI_render.h"
#include "ADM_ffmpeg_xvba_internal.h"
#include "prefs.h"
#include "ADM_coreXvba/include/ADM_coreXvba.h"
#include "ADM_codecXvba.h"
#include "ADM_threads.h"


static bool         xvbaWorking=true;
static admMutex     surfaceMutex;
static bool         destroyingFlag=false;
static BVector   <void *> destroyedList;
#define aprintf(...) {}

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

// dummy
decoderFFXVBA::decoderFFXVBA(uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, 
        uint8_t *extraData,uint32_t bpp)
:decoderFF (w,h,fcc,extraDataLen,extraData,bpp)
{
    alive=false;
    xvba=admXvba::createDecoder(w,h);
    if(!xvba) return;
    _context->opaque          = this;
    _context->thread_count    = 1;
    _context->get_buffer      = ADM_XVBAgetBuffer;
    _context->release_buffer  = ADM_XVBAreleaseBuffer ;
    _context->draw_horiz_band = ADM_XVBADraw;
    _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
    WRAP_Open(CODEC_ID_H264)
    // allocate a few render beforehand
    for(int i=0;i<5;i++)
    {
        void *surface=admXvba::allocateSurface(xvba,w,h);
        if(!surface)
        {
            ADM_warning("Cannot allocate surface\n");
            return;
        }
        xvba_render_state *render = (xvba_render_state*)calloc(sizeof(xvba_render_state), 1);
        render->surface=surface;
        //picture_descriptor ??
        //iq_matrix ??
        freeQueue.push(render);

    }
   // alive=true;

}
/**
 * \fn dtor
 */
decoderFFXVBA::~decoderFFXVBA()
{
    if(xvba)
    {
        while(!freeQueue.isEmpty())
        {
            xvba_render_state *r=freeQueue.pop();
            admXvba::destroySurface(xvba,r->surface);
            free(r);
        }
        // delete decoder
        admXvba::destroyDecoder(xvba);
        xvba=NULL;
    }
}
/**
 * \fn uncompress
 * @param in
 * @param out
 * @return 
 */
bool decoderFFXVBA::uncompress (ADMCompressedImage * in, ADMImage * out)
{
    // First let ffmpeg prepare datas...
    xvba_copy=out;
    decode_status=false;

   
    if(!decoderFF::uncompress (in, scratch))
    {
        aprintf("[XVBA] No data from libavcodec\n");
        return 0;
    }
    if(decode_status!=true)
    {
        printf("[XVBA] error in renderDecode\n");
        return false;
    }
    struct vdpau_render_state *rndr = (struct vdpau_render_state *)scratch->GetReadPtr(PLANAR_Y);
    
   // TODO : Read rndr->out
    
    out->Pts=scratch->Pts;
    out->flags=scratch->flags;
    return (bool)decode_status;
    return false;
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
  
  render=(xvba_render_state*)pic->data[0];
  ADM_assert(render);
  for(i=0; i<4; i++)
  {
    pic->data[i]= NULL;
  }
}
/**
    \fn getBuffer
    \brief returns a VDPAU render masquerading as a AVFrame
*/
int decoderFFXVBA::getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
#if 0
    vdpau_render_state * render;
    if(VDPAU->freeQueue.size()==0)
    {
        aprintf("[VDPAU] No more available surface\n");
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
    pic->data[0]=(uint8_t *)render;
    pic->data[1]=(uint8_t *)render;
    pic->data[2]=(uint8_t *)render;
    pic->linesize[0]=0;
    pic->linesize[1]=0;
    pic->linesize[2]=0;
    pic->type=FF_BUFFER_TYPE_USER;
    render->state |= FF_VDPAU_STATE_USED_FOR_REFERENCE;
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
#endif
    return -1;
}
/**
    \fn goOn
    \brief Callback from ffmpeg when a pic is ready to be decoded
*/
void decoderFFXVBA::goOn( const AVFrame *d,int type)
{
   
   struct xvba_render_state *rndr = (struct xvba_render_state *)d->data[0];
   if(!rndr)
   {
       ADM_warning("Bad context\n");
       return;
   }
#if 0
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
#endif    
    return;
}
#endif
// EOF
