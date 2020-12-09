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

// These are legacy presets, deprecated in SDK 10.x.
// Matching new presets, not supported by FFmpeg 4.2.x, are given for reference.
enum FF_NVencPreset
{
  NV_FF_PRESET_DEFAULT=0, // P4
  NV_FF_PRESET_SLOW=1,
  NV_FF_PRESET_MEDIUM=2,
  NV_FF_PRESET_FAST=3,
  NV_FF_PRESET_HP=4, // P1
  NV_FF_PRESET_HQ=5, // P7
  NV_FF_PRESET_BD=6, // P5
  NV_FF_PRESET_LL=7, // P4
  NV_FF_PRESET_LLHP=8, // P1
  NV_FF_PRESET_LLHQ=9, // P7
  NV_FF_PRESET_LOSSLESS=10, // P4
  NV_FF_PRESET_LOSSLESSHP=11 // P1
};

enum FF_NVencProfile
{
#ifdef H265_ENCODER
  NV_FF_PROFILE_MAIN=0,
  NV_FF_PROFILE_MAIN10=1
#else
  NV_FF_PROFILE_BASELINE=0,
  NV_FF_PROFILE_MAIN=1,
  NV_FF_PROFILE_HIGH=2
#endif
};

enum FF_NVencRateControl
{
  NV_FF_RC_AUTO=0, // controlled by preset
  NV_FF_RC_CONSTQP=1,
  NV_FF_RC_CBR=2,
  NV_FF_RC_CBR_LOWDELAY_HQ=3,
  NV_FF_RC_CBR_HQ=4,
  NV_FF_RC_VBR=5,
  NV_FF_RC_VBR_HQ=6
};

// B-frames as references require SDK 8.1 (driver >= 390.77 on Windows)
enum FF_NVencBframeRefMode
{
  NV_FF_BFRAME_REF_DISABLED=0,
  NV_FF_BFRAME_REF_EACH=1, // invalid for H.264
  NV_FF_BFRAME_REF_MIDDLE=2
};

#ifdef H265_ENCODER
#   define NVENC_CONF_DEFAULT \
{ \
  NV_FF_PRESET_HQ, /* preset */ \
  NV_FF_PROFILE_MAIN, /* profile */ \
  NV_FF_RC_AUTO, /* rc_mode */ \
  20,    /* quality */ \
  5000, /* bitrate */ \
  10000, /* max_bitrate */ \
  100,   /* gopsize */ \
  0, /* bframes */ \
  2, /* b_ref_mode */ \
  0, /* lookahead */ \
  0, /* aq_strength */ \
  0, /* spatial_aq */ \
  0, /* temporal_aq */ \
  1  /* weighted_pred */ \
}
#else
#   define NVENC_CONF_DEFAULT \
{ \
  NV_FF_PRESET_HQ, /* preset */ \
  NV_FF_PROFILE_HIGH, /* profile */ \
  NV_FF_RC_AUTO, /* rc_mode */ \
  20,    /* quality */ \
  10000, /* bitrate */ \
  20000, /* max_bitrate */ \
  100,   /* gopsize */ \
  0, /* bframes */ \
  2, /* b_ref_mode */ \
  0, /* lookahead */ \
  0, /* aq_strength */ \
  0, /* spatial_aq */ \
  0, /* temporal_aq */ \
  1  /* weighted_pred */ \
}
#endif

/**
    \class ADM_ffNvEncEncoder
    \brief Wrapper for h264_nvenc and hevc_nvenc encoders in libavcodec
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
virtual const  char        *getFourcc(void);
virtual        uint64_t     getEncoderDelay(void);
};

