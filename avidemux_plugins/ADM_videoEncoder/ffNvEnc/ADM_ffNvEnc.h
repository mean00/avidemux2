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
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "ffnvenc.h"

enum FF_NVencPreset
{
  NV_FF_PRESET_SLOW=1, // dual-pass
  NV_FF_PRESET_MEDIUM=2,
  NV_FF_PRESET_FAST=3,
  NV_FF_PRESET_HP=4,
  NV_FF_PRESET_HQ=5,
  NV_FF_PRESET_BD=6,
  NV_FF_PRESET_LL=7, // dual-pass
  NV_FF_PRESET_LLHP=8, // dual-pass
  NV_FF_PRESET_LLHQ=9 // dual-pass
};

enum FF_NVencProfile
{
  NV_FF_PROFILE_BASELINE=0,
  NV_FF_PROFILE_MAIN=1,
  NV_FF_PROFILE_HIGH=2
};

enum FF_NVencRateControl
{
  NV_FF_RC_AUTO=0, // controlled by preset
  NV_FF_RC_CONSTQP=1,
  NV_FF_RC_CBR=2,
  NV_FF_RC_CBR_LOWDELAY_HQ=3, // dual-pass
  NV_FF_RC_CBR_HQ=4, // dual-pass
  NV_FF_RC_VBR=5,
  NV_FF_RC_VBR_HQ=6 // dual-pass
};

#define NVENC_CONF_DEFAULT \
{ \
  NV_FF_PRESET_HQ, \
  NV_FF_PROFILE_HIGH, \
  NV_FF_RC_AUTO, \
  20, \
  100, \
  0, \
  10000, \
  20000,\
  0 \
}

/**
        \class ADM_ffNvEncEncoder
        \brief Dummy encoder that does nothing

*/
class ADM_ffNvEncEncoder : public ADM_coreVideoEncoderFFmpeg
{
protected:

               uint8_t      *nv12;
               int          nv12Stride;
               uint64_t     frameIncrement;
public:

                           ADM_ffNvEncEncoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~ADM_ffNvEncEncoder();
virtual        bool        configureContext(void);
virtual        bool        setup(void);
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "H264";}
virtual        uint64_t     getEncoderDelay(void);
virtual        bool         isDualPass(void) ;

};

