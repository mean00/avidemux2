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
#include "ffVAEnc_AV1.h"

#define ADM_FFVAENC_RC_CRF 0
#define ADM_FFVAENC_RC_CBR 1
#define ADM_FFVAENC_RC_VBR 2

#define VAENC_AV1_CONF_DEFAULT \
{ \
    100, /* gopsize */ \
    2, /* bframes (not applicable, unused) */ \
    2500, /* bitrate */ \
    5000, /* max_bitrate */ \
    25, /* quality */ \
    0 /* rc_mode */ \
}

/**
    \class ADM_ffVAEncAV1
*/
class ADM_ffVAEncAV1 : public ADM_coreVideoEncoderFFmpeg
{
protected:
                AVBufferRef *hwDeviceCtx;
                AVFrame     *swFrame;
                AVFrame     *hwFrame;

virtual         bool        preEncode(void);
virtual         bool        configureContext(void);

public:
                            ADM_ffVAEncAV1(ADM_coreVideoFilter *src, bool globalHeader);
virtual                     ~ADM_ffVAEncAV1();

virtual         bool        setup(void);
virtual         bool        encode(ADMBitstream *out);
virtual const   char        *getFourcc(void) {return "av01";}

};
