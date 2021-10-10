/***************************************************************************
                          \file ADM_ffv1Encoder.cpp
                          \brief ffv1 lavcodec based 
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
#include "ADM_ffv1Encoder.h"
#include "DIA_factory.h"



ffv1_encoder ffv1config = FFV1_CONF_DEFAULT;

//
/**
        \fn ADM_ffv1Encoder
*/
ADM_ffv1Encoder::ADM_ffv1Encoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src)
{
    printf("[ffv1Encoder] Creating.\n");
    targetPixFrmt=ADM_PIXFRMT_YV12;
}

/**
    \fn setup
*/
bool ADM_ffv1Encoder::setup(void)
{
    AVCodecID id=AV_CODEC_ID_FFV1;
    av_dict_set_int(&_options, "level", 3, 0);
    switch (ffv1config.coder)
    {
        case ADM_FFV1_CODER_GOLOMB:
                av_dict_set(&_options, "coder", "rice", 0);
            break;
        case ADM_FFV1_CODER_RANGE:
                av_dict_set(&_options, "coder", "range_def", 0);
            break;
        default:break;
    }
    switch (ffv1config.context)
    {
        case ADM_FFV1_CONTEXT_SMALL:
                av_dict_set_int(&_options, "context", 0, 0);
            break;
        case ADM_FFV1_CONTEXT_LARGE:
                av_dict_set_int(&_options, "context", 1, 0);
            break;
        default:break;
    }
    switch (ffv1config.threads)
    {
        case ADM_FFV1_THREADS_1:
                av_dict_set_int(&_options, "threads", 1, 0);
            break;
        case ADM_FFV1_THREADS_2:
                av_dict_set_int(&_options, "threads", 2, 0);
            break;
        case ADM_FFV1_THREADS_4:
                av_dict_set_int(&_options, "threads", 4, 0);
            break;
        default:break;
    }
    av_dict_set_int(&_options, "slicecrc", (ffv1config.slicecrc ? 1:0), 0);

    return ADM_coreVideoEncoderFFmpeg::setup(id);
}


/** 
    \fn ~ADM_ffv1Encoder
*/
ADM_ffv1Encoder::~ADM_ffv1Encoder()
{
    printf("[ffv1Encoder] Destroying.\n");
}
/**
    \fn getFourcc
*/
const char * ADM_ffv1Encoder::getFourcc(void)
{
    return "FFV1";

}
/**
    \fn getExtraData

*/
bool ADM_ffv1Encoder::getExtraData(uint32_t *l,uint8_t **d)
{
    *l=_context->extradata_size;
    *d=_context->extradata;
    return true;
};
/**
    \fn encode
*/
bool ADM_ffv1Encoder::encode (ADMBitstream * out)
{
    if(false==preEncode()) return false;

    int r = encodeWrapper(_frame,out);
    if(r<0)
        return false; // encodeWrapper prints a human-readable error message from libavcodec

    out->len=r;
    out->pts=out->dts=image->Pts;
    return true;
}

/**
    \fn ffv1Configure
    \brief Configuration UI for ffv1 encoder
*/
bool ffv1Configure(void)
{
    uint32_t coderM,contextM,threadsM;
    bool slicecrc;
    printf("[ffv1] Configure\n");
    coderM=(uint32_t)ffv1config.coder;
    contextM=(uint32_t)ffv1config.context;
    threadsM=(uint32_t)ffv1config.threads;
    slicecrc=ffv1config.slicecrc;

    const diaMenuEntry coderMenus[2]={
        {ADM_FFV1_CODER_GOLOMB,QT_TRANSLATE_NOOP("ffv1","Golomb-Rice"),NULL},
        {ADM_FFV1_CODER_RANGE,QT_TRANSLATE_NOOP("ffv1","Range Coder"),NULL}
    };
    const diaMenuEntry contextMenus[2]={
        {ADM_FFV1_CONTEXT_SMALL,QT_TRANSLATE_NOOP("ffv1","Small"),NULL},
        {ADM_FFV1_CONTEXT_LARGE,QT_TRANSLATE_NOOP("ffv1","Large"),NULL}
    };
    const diaMenuEntry threadsMenus[3]={
        {ADM_FFV1_THREADS_1,QT_TRANSLATE_NOOP("ffv1","1"),NULL},
        {ADM_FFV1_THREADS_2,QT_TRANSLATE_NOOP("ffv1","2"),NULL},
        {ADM_FFV1_THREADS_4,QT_TRANSLATE_NOOP("ffv1","4"),NULL}
    };

    diaElemMenu      cd(&coderM,  QT_TRANSLATE_NOOP("ffv1","Coder:"),2,coderMenus);
    diaElemMenu      ct(&contextM,QT_TRANSLATE_NOOP("ffv1","Context:"),2,contextMenus);
    diaElemMenu      th(&threadsM,QT_TRANSLATE_NOOP("ffv1","Threads:"),3,threadsMenus);
    diaElemToggle    sc(&slicecrc,QT_TRANSLATE_NOOP("ffv1","Error correction/detection"));
    diaElem *elems[4]={&cd, &ct, &th, &sc};

    if( diaFactoryRun(QT_TRANSLATE_NOOP("ffv1","FFV1 Configuration"),4 ,elems))
    {
        ffv1config.coder=coderM;
        ffv1config.context=contextM;
        ffv1config.threads=threadsM;
        ffv1config.slicecrc=slicecrc;
        return false;
    }
    return true;
}
// EOF
