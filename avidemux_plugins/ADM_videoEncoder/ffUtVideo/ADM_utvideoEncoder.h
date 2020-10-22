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
#ifndef ADM_UTVIDEO_ENCODER_H
#define ADM_UTVIDEO_ENCODER_H
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "utvideo_encoder.h"

extern utvideo_encoder utconfig;

enum
{
    ADM_UTVIDEO_PREDICTION_NONE,
    ADM_UTVIDEO_PREDICTION_LEFT,
    ADM_UTVIDEO_PREDICTION_MEDIAN
};

#define UTVIDEO_CONF_DEFAULT {ADM_UTVIDEO_PREDICTION_MEDIAN}

/**
    \class ADM_utvideoEncoder
    \brief Wrapper for the libavcodec Ut Video encoder
*/
class ADM_utvideoEncoder : public ADM_coreVideoEncoderFFmpeg
{
public:
                        ADM_utvideoEncoder(ADM_coreVideoFilter *src,bool globalHeader);
                        ~ADM_utvideoEncoder();
virtual         bool    setup(void);
virtual         bool    encode(ADMBitstream *out);
virtual   const char    *getFourcc(void);
virtual         bool    getExtraData(uint32_t *l,uint8_t **d);
};

#endif
