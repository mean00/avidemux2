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
#include "ADM_lavcodec.h"
#include "ADM_default.h"
#include "ADM_jpegEncoder.h"
#include "DIA_factory.h"
jpeg_encoder jpegConf= {ADM_COLOR_YV12,2};
/**
        \fn ADM_jpegEncoder
*/
ADM_jpegEncoder::ADM_jpegEncoder(ADM_coreVideoFilter *src) : ADM_coreVideoEncoderFFmpeg(src)
{
    printf("[jpegEncoder] Creating.\n");
    targetColorSpace=(ADM_colorspace)jpegConf.colorSpace;
}
/**
    \fn setup
*/
bool ADM_jpegEncoder::setup(void)
{
    return ADM_coreVideoEncoderFFmpeg::setup(CODEC_ID_MJPEG);
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
    _context->flags |= CODEC_FLAG_QSCALE;
    _frame.quality = (int) floor (FF_QP2LAMBDA * jpegConf.quantizer+ 0.5);
    int sz=0;
    if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, &_frame)) < 0)
    {
        ADM_error("[jpeg] Error %d encoding video\n",sz);
        return false;
    }
    
    out->len=sz;
    out->pts=out->dts=image->Pts;
    out->flags=AVI_KEY_FRAME;
    return true;
}
/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/
static const diaMenuEntry colorMenus[2]=
    {
	{ADM_COLOR_YUV422P,QT_TR_NOOP("YUV422")},
	{ADM_COLOR_YV12,QT_TR_NOOP("YUV420")},
};
/**
    \fn jpegConfigure
*/
bool         jpegConfigure(void)
{
uint32_t colorM;
    printf("[jpeg] Configure\n");
    colorM=(uint32_t)jpegConf.colorSpace;

    diaElemUInteger  q(&(jpegConf.quantizer),QT_TR_NOOP("_Quantizer:"),2,31);
    diaElemMenu      c(&colorM,QT_TR_NOOP("_ColorSpace:"),2,colorMenus);

    diaElem *elems[2]={&q,&c};
    
  if( diaFactoryRun(QT_TR_NOOP("Mjpeg Configuration"),2 ,elems))
  {
    jpegConf.colorSpace=colorM;
    return false;
  }
  return true;
}
// EOF
