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

enum FF_NVencTune
{
  NV_FF_TUNE_HQ = 1,
#if defined(H265_ENCODER) || defined(AV1_ENCODER)
  NV_FF_TUNE_UHQ = 5,
#endif
  NV_FF_TUNE_LL = 2,
  NV_FF_TUNE_ULL = 3,
  NV_FF_TUNE_LOSSLESS = 4
};

enum FF_NVencPreset
{
  NV_FF_PRESET_P1 = 3,        // fastest (lowest quality)
  NV_FF_PRESET_P2 = 4,        // faster (lower quality)
  NV_FF_PRESET_P3 = 5,        // fast (low quality)
  NV_FF_PRESET_P4 = 6,        // medium (default)
  NV_FF_PRESET_P5 = 7,        // slow (good quality)
  NV_FF_PRESET_P6 = 8,        // slower (better quality)
  NV_FF_PRESET_P7 = 9         // slowest (best quality)
};

enum FF_NVencProfile
{
#ifdef H265_ENCODER
  NV_FF_PROFILE_MAIN=0,
  NV_FF_PROFILE_MAIN10=1
#elif defined(AV1_ENCODER)
  // AV1 does not have profile options
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
  NV_FF_RC_VBR=5
};

// B-frames as references require SDK 8.1 (driver >= 390.77 on Windows) and Turing+
enum FF_NVencBframeRefMode
{
  NV_FF_BFRAME_REF_DISABLED=0,
  NV_FF_BFRAME_REF_EACH=1,
  NV_FF_BFRAME_REF_MIDDLE=2
};

#ifdef H265_ENCODER
#   define NVENC_CONF_DEFAULT \
{ \
  NV_FF_PRESET_P4, /* preset */ \
  NV_FF_PROFILE_MAIN, /* profile */ \
  NV_FF_TUNE_HQ, /* tune */ \
  NV_FF_RC_AUTO, /* rc_mode */ \
  20,    /* quality */ \
  5000, /* bitrate */ \
  10000, /* max_bitrate */ \
  100,   /* gopsize */ \
  0, /* refs */ \
  2, /* bframes */ \
  2, /* b_ref_mode */ \
  0, /* lookahead */ \
  8, /* aq_strength */ \
  0, /* spatial_aq */ \
  0, /* temporal_aq */ \
  0  /* weighted_pred */ \
}
#elif defined(AV1_ENCODER)
#   define NVENC_CONF_DEFAULT \
{ \
  NV_FF_PRESET_P4, /* preset */ \
  0, /* AV1 does not have profile options */ \
  NV_FF_TUNE_HQ, /* tune */ \
  NV_FF_RC_AUTO, /* rc_mode */ \
  25,    /* quality */ \
  5000, /* bitrate */ \
  10000, /* max_bitrate */ \
  100,   /* gopsize */ \
  0, /* refs */ \
  2, /* bframes */ \
  2, /* b_ref_mode */ \
  0, /* lookahead */ \
  8, /* aq_strength */ \
  0, /* spatial_aq */ \
  0, /* temporal_aq */ \
  0  /* weighted_pred */ \
}
#else
#   define NVENC_CONF_DEFAULT \
{ \
  NV_FF_PRESET_P4, /* preset */ \
  NV_FF_PROFILE_HIGH, /* profile */ \
  NV_FF_TUNE_HQ,       /* tune */ \
  NV_FF_RC_AUTO, /* rc_mode */ \
  20,    /* quality */ \
  10000, /* bitrate */ \
  20000, /* max_bitrate */ \
  100,   /* gopsize */ \
  0, /* refs */ \
  2, /* bframes */ \
  1, /* b_ref_mode */ \
  0, /* lookahead */ \
  8, /* aq_strength */ \
  0, /* spatial_aq */ \
  0, /* temporal_aq */ \
  0  /* weighted_pred */ \
}
#endif

/**
    \class ADM_ffNvEncEncoder
    \brief Wrapper for h264_nvenc and hevc_nvenc encoders in libavcodec
*/
class ADM_ffNvEncEncoder : public ADM_coreVideoEncoderFFmpeg
{
protected:
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

