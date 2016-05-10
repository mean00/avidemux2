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
    targetColorSpace=(ADM_colorspace)jpegConf.colorSpace;
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
    _context->flags |= CODEC_FLAG_QSCALE;
    _frame->quality = (int) floor (FF_QP2LAMBDA * jpegConf.quantizer+ 0.5);
    
    if(false==preEncode()) 
        return false;
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
static const diaMenuEntry colorMenus[2]=
    {
	{ADM_COLOR_YUV422P,QT_TRANSLATE_NOOP("jpeg","YUV422")},
	{ADM_COLOR_YV12,QT_TRANSLATE_NOOP("jpeg","YUV420")},
};
/**
    \fn jpegConfigure
*/
bool         jpegConfigure(void)
{
uint32_t colorM;
    printf("[jpeg] Configure\n");
    colorM=(uint32_t)jpegConf.colorSpace;

    diaElemUInteger  q(&(jpegConf.quantizer),QT_TRANSLATE_NOOP("jpeg","_Quantizer:"),2,31);
    diaElemMenu      c(&colorM,QT_TRANSLATE_NOOP("jpeg","_ColorSpace:"),2,colorMenus);

    diaElem *elems[2]={&q,&c};
    
  if( diaFactoryRun(QT_TRANSLATE_NOOP("jpeg","Mjpeg Configuration"),2 ,elems))
  {
    jpegConf.colorSpace=colorM;
    return false;
  }
  return true;
}
// EOF
