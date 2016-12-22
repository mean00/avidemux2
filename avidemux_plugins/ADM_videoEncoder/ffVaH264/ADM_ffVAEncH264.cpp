/***************************************************************************
                          \fn ADM_ffVaEnc
                          \brief Front end for libavcodec Mpeg4 asp encoder
                             -------------------

    copyright            : (C) 2002/2009 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_ffVAEncH264.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
//#define USE_NV12
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

extern "C"
{
    #include "libavutil/opt.h"
#include "libavutil/hwcontext.h"
}

ffvaenc_encoder VaEncSettings;

/**
        \fn ADM_ffVaEncEncoder
*/
ADM_ffVAEncH264Encoder::ADM_ffVAEncH264Encoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src,NULL,globalHeader)
{
    //targetColorSpace=ADM_COLOR_YUV422P;
    ADM_info("[ADM_ffVAEncH264Encoder] Creating.\n");
    surface=NULL;
}

/**
    \fn pre-open
*/
bool ADM_ffVAEncH264Encoder::configureContext(void)
{
#if 0
    switch(VaEncSettings.preset)
    {
#define MIAOU(x,y) case NV_FF_PRESET_##x: ADM_assert(!av_opt_set(_context->priv_data,"preset",y, 0));break;

     MIAOU(HP,"hp")
     MIAOU(BD,"bd")
     MIAOU(LL,"ll")
     MIAOU(LLHP,"llhp")
     MIAOU(LLHQ,"llhq")
     MIAOU(HQ,"hq")
default:break;
    }
#endif
    int err = av_hwdevice_ctx_create(&(_context->hw_frames_ctx), AV_HWDEVICE_TYPE_VAAPI,   "", NULL, 0);
    if(err)        
    {
        ADM_warning("Cannot initialize VAAPI hwdevice (%d)\n",err);
        return false;
    }    
    //
    _context->bit_rate=VaEncSettings.bitrate*1000;
    _context->rc_max_rate=VaEncSettings.max_bitrate*1000;
    _context->pix_fmt =AV_PIX_FMT_VAAPI_VLD;
    
    _frame->format=AV_PIX_FMT_VAAPI_VLD;
    return true;
}

/**
    \fn setup
*/
bool ADM_ffVAEncH264Encoder::setup(void)
{
//nvenc
    surface=ADM_vaSurface::allocateWithSurface(source->getInfo()->width, source->getInfo()->height);
    if(!surface)
    {
        ADM_warning("ffVaEnc : Cannot allocate surface\n");
        return false;
    }
    if(false== ADM_coreVideoEncoderFFmpeg::setupByName("h264_vaapi"))
    {
        ADM_info("[ffMpeg] Setup failed\n");
        return false;
    }

    ADM_info("[ffMpeg] Setup ok\n");

    return true;
}


/**
    \fn ~ADM_ffVaEncEncoder
*/
ADM_ffVAEncH264Encoder::~ADM_ffVAEncH264Encoder()
{
    ADM_info("[ffVaEncEncoder] Destroying.\n");
    if(surface)
    {
        delete surface;
        surface=NULL;
    }
}
/**
 */
bool             ADM_ffVAEncH264Encoder::preEncode(void)
{
    uint32_t nb;
    if(source->getNextFrame(&nb,image)==false)
    {
        printf("[ff] Cannot get next image\n");
        return false;
    }
    prolog(image);

    uint64_t p=image->Pts;
    queueOfDts.push_back(p);
    aprintf("Incoming frame PTS=%" PRIu64", delay=%" PRIu64"\n",p,getEncoderDelay());
    p+=getEncoderDelay();
    _frame->pts= timingToLav(p);    //
    if(!_frame->pts) _frame->pts=AV_NOPTS_VALUE;

    ADM_timeMapping map; // Store real PTS <->lav value mapping
    map.realTS=p;
    map.internalTS=_frame->pts;
    mapper.push_back(map);

    aprintf("Codec> incoming pts=%" PRIu64"\n",image->Pts);
    //printf("--->>[PTS] :%"PRIu64", raw %"PRIu64" num:%"PRIu32" den:%"PRIu32"\n",_frame->pts,image->Pts,_context->time_base.num,_context->time_base.den);
    
    if(!surface->fromAdmImage(image))
    {
        ADM_warning("Cannot upload to vaSurface\n");
        return false;
    }
    
    _frame->data[0] = _frame->data[1] = _frame->data[2] = NULL;
    _frame->data[3]=(uint8_t *)surface->surface;
    
  
    return true;
}
/**
    \fn encode
*/
bool         ADM_ffVAEncH264Encoder::encode (ADMBitstream * out)
{
int sz,q;
again:
    sz=0;
    if(false==preEncode()) // Pop - out the frames stored in the queue due to B-frames
    {
        sz=encodeWrapper(NULL,out);
        if (sz<= 0)
        {
            ADM_info("[ffnvenc] Error %d encoding video\n",sz);
            return false;
        }
        ADM_info("[ffnvenc] Popping delayed bframes (%d)\n",sz);
        goto link;
        return false;
    }
    q=image->_Qp;

    if(!q) q=2;
    aprintf("[CODEC] Flags = 0x%x, QSCALE=%x, bit_rate=%d, quality=%d qz=%d incoming qz=%d\n",_context->flags,CODEC_FLAG_QSCALE,
                                     _context->bit_rate,  _frame->quality, _frame->quality/ FF_QP2LAMBDA,q);

    _frame->reordered_opaque=image->Pts;
    _frame->width=image->GetWidth(PLANAR_Y);
    _frame->height=image->GetHeight(PLANAR_Y);

    sz=encodeWrapper(_frame,out);
    if(sz<0)
    {
        ADM_warning("[ffnvenc] Error %d encoding video\n",sz);
        return false;
    }

    if(sz==0) // no pic, probably pre filling, try again
        goto again;
link:
    return postEncode(out,sz);
}

/**
    \fn isDualPass

*/
bool         ADM_ffVAEncH264Encoder::isDualPass(void)
{
    return false;
}

/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/

bool         ffVAEncConfigure(void)
{
#if 0
diaMenuEntry mePreset[]={
  {NV_FF_PRESET_HP,QT_TRANSLATE_NOOP("ffnvenc","Low Quality")},
  {NV_FF_PRESET_HQ,QT_TRANSLATE_NOOP("ffnvenc","High Quality")},
  {NV_FF_PRESET_BD,QT_TRANSLATE_NOOP("ffnvenc","BluRay")},
  {NV_FF_PRESET_LL,QT_TRANSLATE_NOOP("ffnvenc","Low Latency")},
  {NV_FF_PRESET_LLHP,QT_TRANSLATE_NOOP("ffnvenc","Low Latency (LQ)")},
  {NV_FF_PRESET_LLHQ,QT_TRANSLATE_NOOP("ffnvenc","Low Latency (HQ)")}
};

        ffnvenc_encoder *conf=&NvEncSettings;

#define PX(x) &(conf->x)

        diaElemMenu      qzPreset(PX(preset),QT_TRANSLATE_NOOP("ffnvenc","Preset:"),6,mePreset);
        diaElemUInteger  bitrate(PX(bitrate),QT_TRANSLATE_NOOP("ffnvenc","Bitrate (kbps):"),1,50000);
        diaElemUInteger  maxBitrate(PX(max_bitrate),QT_TRANSLATE_NOOP("ffnvenc","Max Bitrate (kbps):"),1,50000);
          /* First Tab : encoding mode */
        diaElem *diamode[]={&qzPreset,&bitrate,&maxBitrate};

        if( diaFactoryRun(QT_TRANSLATE_NOOP("ffnvenc","libavcodec MPEG-4 configuration"),3,diamode))
        {

          return true;
        }
         return false;
#endif
return true;
}
// EOF
