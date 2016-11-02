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
#include "libavcodec/dxva2_internal.h"
#include "libavcodec/dxva2.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_render/GUI_render.h"
#include "ADM_ffmpeg_dxva2_internal.h"
#include "prefs.h"
#include "ADM_coreDxva2.h"
#include "../private_inc/ADM_codecDxva2.h"
#include "ADM_threads.h"
#include "ADM_vidMisc.h"
#include "prefs.h"


static bool         dxva2Working=false;
static int  ADM_DXVA2getBuffer(AVCodecContext *avctx, AVFrame *pic,int flags);
static void ADM_DXVA2releaseBuffer(struct AVCodecContext *avctx, AVFrame *pic);



#if 0
#define aprintf(...) {}
#else
#define aprintf printf
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
        return false;
    }

    if(false==admDxva2::init(&xinfo)) return false;
    dxva2Working=true;
    return true;
}
/**
    \fn ADM_DXVA2getBuffer
    \brief trampoline to get a DXVA2 surface
*/
int ADM_DXVA2getBuffer(AVCodecContext *avctx, AVFrame *pic,int flags)
{
    decoderFF *ff=(decoderFF *)avctx->opaque;
    decoderFFDXVA2 *dec=(decoderFFDXVA2 *)ff->getHwDecoder();
    ADM_assert(dec);
    return dec->getBuffer(avctx,pic);
}

/**
 * \fn ADM_XVBAreleaseBuffer
 * @param avctx
 * @param pic
 */
void ADM_LIBDXVA2releaseBuffer(void *opaque, uint8_t *data)
{
    
   admDx2Surface *w=(admDx2Surface *)opaque;
   aprintf("=> Release Buffer %p\n",w);   
   decoderFFDXVA2 *instance=w->admClass;
   instance->releaseBuffer(w);
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
        
        if(c!=AV_PIX_FMT_DXVA2_VLD) continue;
#define FMT_V_CHECK(x,y)      case AV_CODEC_ID_##x:   outPix=AV_PIX_FMT_DXVA2_VLD;id=avctx->codec_id;break;
        
        
        switch(avctx->codec_id)  //AV_CODEC_ID_H265
        {
            FMT_V_CHECK(H264,H264)
            FMT_V_CHECK(H265,H265)
//            FMT_V_CHECK(VC1,VC1)
//            FMT_V_CHECK(VP9,VP9)
            default: 
                ADM_info("DXVA2 No hw support for format %d\n",avctx->codec_id);
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
    const AVHWAccel *accel=ADM_acceleratedDecoderFF::parseHwAccel(outPix,id,AV_PIX_FMT_DXVA2_VLD);
    if(accel)
    {
        ADM_info("Found matching hw accelerator : %s\n",accel->name);
        ADM_info("Successfully setup hw accel\n");
        return AV_PIX_FMT_DXVA2_VLD;
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
decoderFFDXVA2::decoderFFDXVA2(AVCodecContext *avctx,decoderFF *parent)
: ADM_acceleratedDecoderFF(avctx,parent)
{
    
    ADM_info("Setting up Dxva2 context (not working)\n");
    alive=false;
    _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
    _context->thread_count    = 1;
    //
    memset(surfaces,0,sizeof(*surfaces)*ADM_MAX_SURFACE); // should not be here...
    // create decoder
    AVDXVAContext *dx_context=new AVDXVAContext;
    memset(dx_context,0,sizeof(*dx_context)); // dangerous...    
    
    // Allocate temp buffer
    num_surfaces=4;
    int align;
    switch(avctx->codec_id)
    {
        case AV_CODEC_ID_H265:
                align=128;
                num_surfaces+=16;
                break;
        case AV_CODEC_ID_H264:
                align=16;
                num_surfaces+=16;
                break;                
        case AV_CODEC_ID_VP9:                
                num_surfaces+=8;
                break;
        default:
                align=16;
                num_surfaces+=2;
                break;

    }
#define ALIGN(x) ((x+(align-1)) &(~(align-1)))
    // Allocate surfaces..
    if(!admDxva2::allocateD3D9Surface(num_surfaces,ALIGN(avctx->coded_width),ALIGN(avctx->coded_height),surfaces))
    {
        ADM_warning("Cannot allocate surfaces \n");
        return ;
    }
    //
    _context->opaque=this;
    _context->hwaccel_context=dx_context;
    _context->get_buffer2     = ADM_DXVA2getBuffer;    
    _context->draw_horiz_band = NULL;
    ADM_info("Successfully setup DXVA2 hw accel\n");             
    alive=true;
}

/**
 * \fn dtor
 */
decoderFFDXVA2::~decoderFFDXVA2()
{
    if(alive)
        admDxva2::destroyD3DSurface(num_surfaces,surfaces);
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
    return false;
}
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
 */
int decoderFFDXVA2::getBuffer(AVCodecContext *avctx, AVFrame *frame)
{
    aprintf("-> Get buffer\n");
    int i, old_unused = -1;
    LPDIRECT3DSURFACE9 surface;
    admDx2Surface *w = NULL;

    ADM_assert(frame->format == AV_PIX_FMT_DXVA2_VLD);
    for (i = 0; i < num_surfaces; i++)
    {
        surface_info *info = &surface_infos[i];
        if (!info->used && (old_unused == -1 || info->age < surface_infos[old_unused].age))
            old_unused = i;
    }
    if (old_unused == -1)
    {
        ADM_warning("No free DXVA2 surface!\n");
        return AVERROR(ENOMEM);
    }
    i = old_unused;

    surface = surfaces[i];

    w = new admDx2Surface(this);
    frame->buf[0] = av_buffer_create((uint8_t*)surface, 0,
                                     ADM_LIBDXVA2releaseBuffer, w,
                                     AV_BUFFER_FLAG_READONLY);
    if (!frame->buf[0])
    {
        av_free(w);
        return AVERROR(ENOMEM);
    }

    w->surface   = surface;
  // FIXME  w->decoder   = decoder;
    
    w->surface->addRef();
#warning TODO    
   // FIXME  IDirectXVideoDecoder_AddRef(w->decoder);

    surface_infos[i].used = 1;
    surface_infos[i].age  = surface_age++;

    frame->data[3] = (uint8_t *)surface;
    frame->data[1] = (uint8_t *)w;
    aprintf("   <= Got buffer %p\n",w);
    return 0;
}
/**
 * \fn releaseBuffer
 */
bool decoderFFDXVA2::releaseBuffer(admDx2Surface *surface)
{
   bool found=false;
   for (int i = 0; i < num_surfaces; i++) 
   {
        if (surfaces[i] == surface->surface) 
        {
            found=true;
            surface_infos[i].used = 0;
            break;
        }
    }
    ADM_assert(found);
    surface->removeRef();
#warning TODO    
    //IDirectXVideoDecoder_Release(surface->decoder);
    delete surface;
    surface=NULL;
    return true ;
}

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

    outputFormat=ofmt;
    ADM_info("This is maybe supported by DXVA2...\n");
    if(!admDxva2::supported(avctx->codec_id))
    {
        ADM_warning("Not supported by DXVA2\n");
        return false;
    }
    return true;
}


/**
 * 
 * @param avctx
 * @param fmt
 * @return 
 */
ADM_acceleratedDecoderFF *ADM_hwAccelEntryDxva2::spawn( struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt )
{
    decoderFF *ff=(decoderFF *)avctx->opaque;
    return new decoderFFDXVA2(avctx,ff);
}
static ADM_hwAccelEntryDxva2 dxva2Entry;
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
