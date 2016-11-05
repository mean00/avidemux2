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
#define CONFIG_DXVA2 1
#include "libavcodec/dxva2_internal.h"
#include "libavcodec/dxva2.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_render/GUI_render.h"
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
    aprintf("Get Buffer: FF avctx=%p parent=%p instance=%p\n",avctx,ff,dec);
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
   decoderFFDXVA2 *instance=(decoderFFDXVA2 *)w->parent;
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
    
    ADM_info("Setting up Dxva2 context (instance=%p)\n",this);
    alive=false;
    _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
    _context->thread_count    = 1;
    //
    for(int i=0;i<ADM_MAX_SURFACE;i++)
    {
        surfaces[i]=NULL;
        surface_infos[i].used = 0;
        surface_infos[i].age  = 0;
    }
    surface_age=1;

    // create decoder
    struct dxva_context *dx_context=new struct dxva_context;
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
                align=16;
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
    dx_context->decoder=admDxva2::createDecoder(avctx->codec_id,ALIGN(avctx->coded_width), ALIGN(avctx->coded_height),num_surfaces,surfaces);
    if(!dx_context->decoder)
    {
        ADM_warning("Cannot create decoder\n");
        return ;
    }
    ADM_info("DXVA2 decoder created\n");
    
    //
    _context->hwaccel_context = dx_context;
    _context->get_buffer2     = ADM_DXVA2getBuffer;    
    _context->draw_horiz_band = NULL;
    
    dx_context->surface         = surfaces;
    dx_context->surface_count   = num_surfaces;
    dx_context->cfg             = admDxva2::getDecoderConfig(avctx->codec_id);
    
    ADM_info("Ctor Successfully setup DXVA2 hw accel (%d surface created, ffdxva=%p,parent=%p,context=%p)\n",num_surfaces,this,parent,avctx);
    
    alive=true;
}

/**
 * \fn dtor
 */
decoderFFDXVA2::~decoderFFDXVA2()
{
    if(alive)
        admDxva2::destroyD3DSurface(num_surfaces,surfaces);
    if(_context->hwaccel_context)
    {
        admDxva2::destroyDecoder((IDirectXVideoDecoderService *)_context->hwaccel_context);
        _context->hwaccel_context=NULL;
    }
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
    if (!in->dataLength )	// Null frame, silently skipped
    {
        out->_noPicture = 1;
        out->Pts=ADM_COMPRESSED_NO_PTS;
        out->refType=ADM_HW_NONE;
        ADM_info("[dxva] Nothing to decode -> no Picture\n");
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
        ADM_warning("DXVA: Error %d in lavcodec (%s)\n",ret,er);
        out->refType=ADM_HW_NONE;
        return false;
    }
    if(frame->pict_type==AV_PICTURE_TYPE_NONE)
    {
        out->_noPicture=true;
        out->refType=ADM_HW_NONE;
        out->Pts= (uint64_t)(frame->reordered_opaque);
        ADM_info("[DXVA] No picture \n");
        return false;
    }
    ADM_warning("Todo : Read back image\n");
    return true;
}
//---

/**
 */
int decoderFFDXVA2::getBuffer(AVCodecContext *avctx, AVFrame *frame)
{
    aprintf("-> Get buffer (%d,%p)\n",num_surfaces,this);
    int i, old_unused = -1;
    LPDIRECT3DSURFACE9 surface;
    admDx2Surface *w = NULL;
    int older=surface_age;
    ADM_assert(frame->format == AV_PIX_FMT_DXVA2_VLD);
    for (i = 0; i < num_surfaces; i++)
    {
        surface_info *info = &surface_infos[i];
        if(info->used)
        {
            aprintf("Surface %d is busy\n",i);
            continue;
        }
        
        if(info->age<older)
        {
            old_unused=i;
            older=surface_infos[i].age;
        }
    }
    if (old_unused == -1)
    {
        ADM_warning("No free DXVA2 surface!\n");
        return AVERROR(ENOMEM);
    }
    i = old_unused;

    surface = surfaces[i];
    aprintf("-> found surface\n");
    
    w = new admDx2Surface(this);
    frame->buf[0] = av_buffer_create((uint8_t*)surface, 0,
                                     ADM_LIBDXVA2releaseBuffer, w,
                                     AV_BUFFER_FLAG_READONLY);
    if (!frame->buf[0])
    {
        av_free(w);
        return AVERROR(ENOMEM);
    }

    aprintf("-> adding ref\n");
    w->surface   = surface;
    struct dxva_context * hwContext=(struct dxva_context *) _context->hwaccel_context;
    w->decoder   = hwContext->decoder;
    w->addRef();
#warning TODO    
   // FIXME  IDirectXVideoDecoder_AddRef(w->decoder);

    surface_infos[i].used = 1;
    surface_infos[i].age  = surface_age++; // not sure...
    frame->data[3] = (uint8_t *)surface;
    aprintf("   <= Got buffer %p\n",w);
    return 0;
}
/**
 * \fn releaseBuffer
 */
bool decoderFFDXVA2::releaseBuffer(admDx2Surface *surface)
{
   bool found=false;
   aprintf("->Release Buffer\n");
   for (int i = 0; i < num_surfaces; i++) 
   {
        if (surfaces[i] == surface->surface) 
        {
            found=true;
            aprintf("   match %d\n",i);
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
 */
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
    decoderFFDXVA2 *instance= new decoderFFDXVA2(avctx,ff);
    if(!instance->isAlive())
    {
                ADM_warning("DXVA2 decoder is not alive, killing it\n");
                delete instance;
                instance=NULL;
    }else
    {
        ADM_info("Spawned DXVA2 decoder with %d surfaces, avcontext=%p, ff=%p, instance=%p\n",instance->getNumSurfaces(),
                                avctx,ff,instance);
    }
    return instance;
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
