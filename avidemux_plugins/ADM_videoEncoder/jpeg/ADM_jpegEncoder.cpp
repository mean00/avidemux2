/***************************************************************************
                          \fn ADM_VideoEncoders
                          \brief Internal handling of video encoders
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
#include "ADM_jpegEncoder.h"
#include "DIA_factory.h"



jpeg_encoder jpegConf= JPEG_CONF_DEFAULT;
/**
        \fn ADM_jpegEncoder
*/
ADM_jpegEncoder::ADM_jpegEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src)
{
    printf("[jpegEncoder] Creating.\n");
    targetPixFrmt=(ADM_pixelFormat)jpegConf.pixelFormat;
}

/**
    \fn configureContext
*/
bool ADM_jpegEncoder::configureContext(void)
{
    _context->flags |= AV_CODEC_FLAG_QSCALE;
    return true;
}

/**
    \fn setup
*/
bool ADM_jpegEncoder::setup(void)
{
    return ADM_coreVideoEncoderFFmpeg::setup(AV_CODEC_ID_MJPEG);
}


/** 
    \fn ~ADM_jpegEncoder
*/
ADM_jpegEncoder::~ADM_jpegEncoder()
{
    ADM_info("[jpegEncoder] Destroying.\n");
    
}

/**
    \fn encode
*/
bool         ADM_jpegEncoder::encode (ADMBitstream * out)
{
    if(false==preEncode()) return false;
    _frame->quality = (int)jpegConf.quantizer * FF_QP2LAMBDA;

    int sz=0,r,gotData;
    r=encodeWrapper(_frame,out);
    if(r<0)
    {
        ADM_warning("[ffHuff] Error %d encoding video\n",r);
        return false;
    }
    sz=r;    
    
    out->len=sz;
    out->pts=out->dts=image->Pts;
    out->flags=AVI_KEY_FRAME;
    return true;
}
/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/
bool         jpegConfigure(void)
{
uint32_t pixfrmtM;
    printf("[jpeg] Configure\n");
    pixfrmtM=(uint32_t)jpegConf.pixelFormat;

    const diaMenuEntry pixfrmtMenus[2]={
        {ADM_PIXFRMT_YUV422P,QT_TRANSLATE_NOOP("jpeg","YUV422"),NULL},
        {ADM_PIXFRMT_YV12,QT_TRANSLATE_NOOP("jpeg","YUV420"),NULL}
    };

    diaElemUInteger  q(&(jpegConf.quantizer),QT_TRANSLATE_NOOP("jpeg","_Quantizer:"),2,31);
    diaElemMenu      c(&pixfrmtM,QT_TRANSLATE_NOOP("jpeg","_Pixel format:"),2,pixfrmtMenus);

    diaElem *elems[2]={&q,&c};
    
  if( diaFactoryRun(QT_TRANSLATE_NOOP("jpeg","Mjpeg Configuration"),2 ,elems))
  {
    jpegConf.pixelFormat=pixfrmtM;
    return false;
  }
  return true;
}
// EOF
