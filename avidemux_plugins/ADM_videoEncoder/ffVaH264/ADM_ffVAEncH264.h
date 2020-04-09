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
#pragma once
#include "ADM_coreLibVA/ADM_coreLibVA.h"
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "ffVAEnc_H264.h"

#define ADM_FFVAENC_RC_CRF 0
#define ADM_FFVAENC_RC_CBR 1
#define ADM_FFVAENC_RC_VBR 2

#define VAENC_CONF_DEFAULT \
{ \
    FF_PROFILE_H264_HIGH, \
    100, \
    2, \
    4000, \
    8000, \
    20, \
    0 \
}

/**
        \class ADM_ffVAEncH264Encoder
        \brief

*/
class ADM_ffVAEncH264Encoder : public ADM_coreVideoEncoderFFmpeg
{
public:
                           ADM_ffVAEncH264Encoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~ADM_ffVAEncH264Encoder();
virtual        bool        configureContext(void);
virtual        bool        setup(void);
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "H264";}
virtual        uint64_t    getEncoderDelay(void);
virtual        bool        isDualPass(void) ;

protected:
                AVBufferRef *hwDeviceCtx;
                AVFrame    *swFrame;
                AVFrame    *hwFrame;
                bool       preEncode(void);
};
