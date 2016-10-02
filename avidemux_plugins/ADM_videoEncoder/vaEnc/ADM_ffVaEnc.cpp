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
#include "ADM_ffVaEnc.h"
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
}

ffvaenc_encoder VaEncSettings = VAENC_CONF_DEFAULT;

/**
        \fn ADM_ffVaEncEncoder
*/
ADM_ffVaEncEncoder::ADM_ffVaEncEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src,NULL,globalHeader)
{
    //targetColorSpace=ADM_COLOR_YUV422P;
    ADM_info("[ffNvEncEncoder] Creating.\n");
    nv12=NULL;


}

/**
    \fn pre-open
*/
bool ADM_ffVaEncEncoder::configureContext(void)
{
   
    _context->bit_rate   = VaEncSettings.bitrate*1000;
    _context->rc_max_rate= VaEncSettings.max_bitrate*1000;
    _context->pix_fmt    = AV_PIX_FMT_VAAPI_VLD;        
    
    return true;             
}

/**
    \fn setup
*/
bool ADM_ffVaEncEncoder::setup(void)
{
//nvenc
    if(false== ADM_coreVideoEncoderFFmpeg::setupByName("h264_vaapi"))
    {
        ADM_info("[vaEncoder] Setup failed\n");
        return false;
    }

    ADM_info("[vaEncoder] Setup ok\n");
    
    int w= getWidth();
    int h= getHeight();
    
    w=(w+31)&~31;
    h=(h+15)&~15;
    
    nv12=new uint8_t[(w*h)/2]; 
    nv12Stride=w;
    
    return true;
}


/**
    \fn ~ADM_ffVaEncEncoder
*/
ADM_ffVaEncEncoder::~ADM_ffVaEncEncoder()
{
    ADM_info("[vaEncoder] Destroying.\n");
    if(nv12)
    {
        delete [] nv12;
        nv12=NULL;
    }

}

/**
    \fn encode
*/
bool         ADM_ffVaEncEncoder::encode (ADMBitstream * out)
{
int sz,q;
again:
    sz=0;
    if(false==preEncode()) // Pop - out the frames stored in the queue due to B-frames
    {
        sz=encodeWrapper(NULL,out);
        if (sz<= 0)
        {
            ADM_info("[vaEncoder] Error %d encoding video\n",sz);
            return false;
        }
        ADM_info("[vaEncoder] Popping delayed bframes (%d)\n",sz);
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

// convert to nv12
#ifdef USE_NV12
    image->interleaveUV(nv12,nv12Stride);
    _frame->data[0] = image->GetReadPtr(PLANAR_Y);
    _frame->data[1] = nv12;
    _frame->data[2] = NULL;

    _frame->linesize[0]=image->GetPitch(PLANAR_Y);
    _frame->linesize[1]=nv12Stride;
    _frame->linesize[2]=0;
    _frame->format=  AV_PIX_FMT_NV12;    
#else
    _frame->format=  AV_PIX_FMT_YUV420P;    
#endif        
    sz=encodeWrapper(_frame,out);
    if(sz<0)
    {
        ADM_warning("[vaEncoder] Error %d encoding video\n",sz);
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
bool         ADM_ffVaEncEncoder::isDualPass(void)
{   
    return false;
}

/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/

bool         ffVaEncConfigure(void)
{

        ffvaenc_encoder *conf=&VaEncSettings;

#define PX(x) &(conf->x)

        
        diaElemUInteger  bitrate(PX(bitrate),QT_TRANSLATE_NOOP("vaenc","Bitrate (kbps):"),1,50000);
        diaElemUInteger  maxBitrate(PX(max_bitrate),QT_TRANSLATE_NOOP("vaenc","Max Bitrate (kbps):"),1,50000);
          /* First Tab : encoding mode */
        diaElem *diamode[]={&bitrate,&maxBitrate};

        if( diaFactoryRun(QT_TRANSLATE_NOOP("vaenc","VAAPI H264 Encoder"),2,diamode))
        {
          
          return true;
        }
         return false;
}
// EOF
