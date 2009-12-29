/***************************************************************************
                          \file ADM_huffEncoder.cpp
                          \brief (FF)Huffyuv lavcodec based 
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
#include "ADM_huffEncoder.h"
#include "DIA_factory.h"

enum
{
    ADM_HUFF_YUV,
    ADM_FF_HUFF_YUV
};
huff_encoder huffType={ADM_HUFF_YUV};
//
/**
        \fn ADM_huffEncoder
*/
ADM_huffEncoder::ADM_huffEncoder(ADM_coreVideoFilter *src) : ADM_coreVideoEncoderFFmpeg(src)
{
    printf("[huffEncoder] Creating.\n");
    if(huffType.encoderType==ADM_HUFF_YUV)
        targetColorSpace=ADM_COLOR_YUV422P;
    else
        targetColorSpace=ADM_COLOR_YV12;
}
/**
    \fn setup
*/
bool ADM_huffEncoder::setup(void)
{
    CodecID id=CODEC_ID_HUFFYUV;
    if(huffType.encoderType==ADM_FF_HUFF_YUV) id=CODEC_ID_FFVHUFF;
    return ADM_coreVideoEncoderFFmpeg::setup(id);
}


/** 
    \fn ~ADM_huffEncoder
*/
ADM_huffEncoder::~ADM_huffEncoder()
{
    printf("[huffEncoder] Destroying.\n");
    
}
/**
    \fn getFourcc
*/
const  char        *ADM_huffEncoder::getFourcc(void)
{
    if(huffType.encoderType==ADM_HUFF_YUV) return "HFYU";
    else return "FFVH";

}
/**
    \fn getExtraData

*/
bool         ADM_huffEncoder::getExtraData(uint32_t *l,uint8_t **d)
{
     *l=_context->extradata_size;
     *d=_context-> extradata;
     return true;
};
/**
    \fn encode
*/
bool         ADM_huffEncoder::encode (ADMBitstream * out)
{
    if(false==preEncode()) return false;
    int sz=0;
    if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, &_frame)) < 0)
    {
        printf("[huff] Error %d encoding video\n",sz);
        return false;
    }
    
    out->len=sz;
    out->pts=out->dts=image->Pts;
    out->flags=AVI_KEY_FRAME;
    return true;
}
/**
    \fn huffConfigure
    \brief UI configuration for huff encoder
*/
    
static const diaMenuEntry colorMenus[2]=
    {
	{ADM_HUFF_YUV,QT_TR_NOOP("HUFFYUV")},
	{ADM_FF_HUFF_YUV,QT_TR_NOOP("FF HUFFYUV")},
};

bool         huffConfigure(void)
{
uint32_t colorM;
    printf("[huff] Configure\n");
    colorM=(uint32_t)huffType.encoderType;

    diaElemMenu      c(&colorM,QT_TR_NOOP("Type:"),2,colorMenus);
    diaElem *elems[1]={&c};
    
      if( diaFactoryRun(QT_TR_NOOP("HuffYUV Configuration"),1 ,elems))
      {
        huffType.encoderType=colorM;
        return false;
      }
      return true;
}
// EOF
