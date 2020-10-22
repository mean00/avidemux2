/***************************************************************************
                        \file  ADM_utvideoEncoder.cpp
                        \brief Wrapper for libavcodec Ut Video encoder
                        ----------------------------------------------
    copyright           : (C) 2002/2020 by mean
    email               : fixounet@free.fr
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
#include "ADM_utvideoEncoder.h"
#include "DIA_factory.h"

utvideo_encoder utconfig = UTVIDEO_CONF_DEFAULT;

/**
    \fn ADM_utvideoEncoder
*/
ADM_utvideoEncoder::ADM_utvideoEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src)
{
    ADM_info("Creating encoder.\n");
}
/**
    \fn setup
*/
bool ADM_utvideoEncoder::setup(void)
{
    const char *method;
    switch(utconfig.prediction)
    {
        case ADM_UTVIDEO_PREDICTION_NONE:
            method = "none";
            break;
        case ADM_UTVIDEO_PREDICTION_LEFT:
            method = "left";
            break;
        case ADM_UTVIDEO_PREDICTION_MEDIAN:
            method = "median";
            break;
        default:
            ADM_warning("Unsupported prediction method requested, using median.\n");
            method = "median";
            break;
    }
    av_dict_set(&_options, "pred", method, 0);
    return ADM_coreVideoEncoderFFmpeg::setup(AV_CODEC_ID_UTVIDEO);
}
/**
    \fn ~ADM_utvideoEncoder
*/
ADM_utvideoEncoder::~ADM_utvideoEncoder()
{
    ADM_info("Destroying Ut Video encoder.\n");
}
/**
    \fn getFourcc
*/
const char *ADM_utvideoEncoder::getFourcc(void)
{
    return "ULY0";
}
/**
    \fn getExtraData

*/
bool ADM_utvideoEncoder::getExtraData(uint32_t *l,uint8_t **d)
{
    *l=_context->extradata_size;
    *d=_context->extradata;
    return true;
};
/**
    \fn encode
*/
bool ADM_utvideoEncoder::encode(ADMBitstream *out)
{
    if(!preEncode())
        return false;

    int sz = encodeWrapper(_frame,out);
    if(sz < 0)
    {
        ADM_warning("[utvideo] Error %d encoding video\n",sz);
        return false;
    }
    out->len=sz;
    out->pts=out->dts=image->Pts;
    out->flags=AVI_KEY_FRAME;
    return true;
}
/**
    \fn utvideoConfigure
    \brief UI configuration for utvideo encoder
*/
bool utvideoConfigure(void)
{
    uint32_t method = utconfig.prediction;

    diaMenuEntry predMethods[] = {
        {ADM_UTVIDEO_PREDICTION_NONE,QT_TRANSLATE_NOOP("utvideo","None")},
        {ADM_UTVIDEO_PREDICTION_LEFT,QT_TRANSLATE_NOOP("utvideo","Left Neighbour")},
        {ADM_UTVIDEO_PREDICTION_MEDIAN,QT_TRANSLATE_NOOP("utvideo","Median")}
    };

    diaElemMenu pm(&method,QT_TRANSLATE_NOOP("utvideo","Prediction Method:"),3,predMethods);
    diaElem *dialog[1] = {&pm};
    if(diaFactoryRun(QT_TRANSLATE_NOOP("utvideo","Ut Video Encoder Configuration"),1,dialog))
    {
        utconfig.prediction = method;
        return true;
    }
    return false;
}
// EOF
