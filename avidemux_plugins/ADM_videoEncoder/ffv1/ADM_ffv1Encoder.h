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
#ifndef ADM_ffv1_ENCODER_H
#define ADM_ffv1_ENCODER_H
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "ffv1_encoder.h"

extern ffv1_encoder ffv1config;

enum
{
    ADM_FFV1_CODER_GOLOMB = 0,
    ADM_FFV1_CODER_RANGE,
};

enum
{
    ADM_FFV1_CONTEXT_SMALL = 0,
    ADM_FFV1_CONTEXT_LARGE,
};

enum
{
    ADM_FFV1_THREADS_1 = 0,
    ADM_FFV1_THREADS_2,
    ADM_FFV1_THREADS_4,
};


#define FFV1_CONF_DEFAULT {0}

/**
        \class ADM_ffv1Encoder
        \brief Dummy encoder that does nothing

*/

class ADM_ffv1Encoder : public ADM_coreVideoEncoderFFmpeg
{
protected:
               int              plane;
public:

                           ADM_ffv1Encoder(ADM_coreVideoFilter *src,bool globalHeader);
                           ~ADM_ffv1Encoder();
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void);
virtual        bool         getExtraData(uint32_t *l,uint8_t **d);
};


#endif
