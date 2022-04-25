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
#include "ffvtenc.h"

enum FF_VTEncProfile
{
#ifdef H265_ENCODER
    FF_VT_HEVC_PROFILE_MAIN=1,
    FF_VT_HEVC_PROFILE_MAIN10=2
#else
    FF_VT_H264_PROFILE_BASELINE=1,
    FF_VT_H264_PROFILE_MAIN=2,
    FF_VT_H264_PROFILE_HIGH=3
#endif
};

#ifdef H265_ENCODER
#define VT_ENC_CONF_DEFAULT \
{ \
    FF_VT_HEVC_PROFILE_MAIN, \
    100, \
    0, \
    2000, \
    4000 \
}
#else
#define VT_ENC_CONF_DEFAULT \
{ \
    FF_VT_H264_PROFILE_HIGH, \
    100, \
    0, \
    2000, \
    4000 \
}
#endif

/**
        \class ADM_ffVTEncoder
*/
class ADM_ffVTEncoder : public ADM_coreVideoEncoderFFmpeg
{
protected:
               uint64_t    frameIncrement;

public:
                           ADM_ffVTEncoder(ADM_coreVideoFilter *src, bool globalHeader);
    virtual                ~ADM_ffVTEncoder();
    virtual    bool        configureContext(void);
    virtual    bool        setup(void);
    virtual    bool        encode(ADMBitstream *out);
    virtual const char     *getFourcc(void);
    virtual    bool        isDualPass(void);
    virtual    uint64_t    getEncoderDelay(void);
};
//EOF