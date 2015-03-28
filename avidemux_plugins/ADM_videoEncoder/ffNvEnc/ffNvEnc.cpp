/***************************************************************************
                          \fn ADM_ffNvEnc
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
#include "ffNvEnc.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
#define USE_NV12 
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

ffnvenc_encoder NvEncSettings = NVENC_CONF_DEFAULT;

/**
        \fn ADM_ffNvEncEncoder
*/
ADM_ffNvEncEncoder::ADM_ffNvEncEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src,NULL,globalHeader)
{
    //targetColorSpace=ADM_COLOR_YUV422P;
    ADM_info("[ffNvEncEncoder] Creating.\n");
    nv12=NULL;


}

/**
    \fn pre-open
*/
bool ADM_ffNvEncEncoder::configureContext(void)
{
    switch(NvEncSettings.preset)
    {
#define MIAOU(x,y) //case NV_FF_PRESET_##x: _context->  preset(y)break;  AVOption options[]
        
     MIAOU(HP,"hp")   
     MIAOU(BD,"bd")   
     MIAOU(LL,"ll")   
     MIAOU(LL,"ll")   
     MIAOU(LLHP,"llhp")   
     MIAOU(LLHQ,"llhq")   
     MIAOU(HQ,"hq")                
default:break;
    }
    _context->bit_rate=NvEncSettings.bitrate*1000;
    _context->rc_max_rate=NvEncSettings.max_bitrate*1000;
#ifdef USE_NV12    
    _context->pix_fmt=  AV_PIX_FMT_NV12;        
#else    
    _context->pix_fmt=AV_PIX_FMT_YUV420P;
#endif    
    
    return true;             
}

/**
    \fn setup
*/
bool ADM_ffNvEncEncoder::setup(void)
{
//nvenc
    if(false== ADM_coreVideoEncoderFFmpeg::setupByName("nvenc"))
    {
        ADM_info("[ffMpeg] Setup failed\n");
        return false;
    }

    ADM_info("[ffMpeg] Setup ok\n");
    
    int w= getWidth();
    int h=getHeight();
    w=(w+15)&~15;
    h=(h+15)&~15;
    nv12=new uint8_t[w*h*2];
    strides[0]=w;
    strides[1]=w;
    planes[0]=nv12;
    planes[1]=nv12+w*h;
    return true;
}


/**
    \fn ~ADM_ffNvEncEncoder
*/
ADM_ffNvEncEncoder::~ADM_ffNvEncEncoder()
{
    ADM_info("[ffNvEncEncoder] Destroying.\n");
    if(nv12)
    {
        delete [] nv12;
        nv12=NULL;
    }

}

/**
    \fn encode
*/
bool         ADM_ffNvEncEncoder::encode (ADMBitstream * out)
{
int sz,q;
again:
    sz=0;
    if(false==preEncode()) // Pop - out the frames stored in the queue due to B-frames
    {
        if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, NULL)) <= 0)
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
// convert to nv12
#ifdef USE_NV12
    image->convertToNV12(planes[0],planes[1],strides[0],strides[1]);
    _frame->data[0] = planes[0];
    _frame->data[1] = planes[1];
    _frame->data[2] = NULL;

    _frame->linesize[0]=strides[0];
    _frame->linesize[1]=strides[1];
    _frame->linesize[2]=0;
    _frame->width=image->GetWidth(PLANAR_Y);
    _frame->height=image->GetHeight(PLANAR_Y);
    _frame->format=  AV_PIX_FMT_NV12;    
#else
    _frame->format=  AV_PIX_FMT_YUV420P;    
#endif        
    if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, _frame)) < 0)
    {
        ADM_warning("[ffnvenc] Error %d encoding video\n",sz);
        return false;
    }

    if(sz==0) // no pic, probably pre filling, try again
        goto again;
link:
    postEncode(out,sz);

    return true;
}

/**
    \fn isDualPass

*/
bool         ADM_ffNvEncEncoder::isDualPass(void)
{   
    return false;
}

/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/

bool         ffNvEncConfigure(void)
{
diaMenuEntry mePreset[]={ 
  {NV_FF_PRESET_HP,QT_TRANSLATE_NOOP("ffnvenc","hp")},
  {NV_FF_PRESET_HQ,QT_TRANSLATE_NOOP("ffnvenc","hq")},
  {NV_FF_PRESET_BD,QT_TRANSLATE_NOOP("ffnvenc","bd")},
  {NV_FF_PRESET_LL,QT_TRANSLATE_NOOP("ffnvenc","ll")},
  {NV_FF_PRESET_LLHP,QT_TRANSLATE_NOOP("ffnvenc","llhp")},
  {NV_FF_PRESET_LLHQ,QT_TRANSLATE_NOOP("ffnvenc","llhq")}
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
}
// EOF
