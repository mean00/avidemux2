/***************************************************************************
                  \fn     ffVTEnc
                  \brief  Front end for the *_videotoolbox lav encoders
                           -------------------

    copyright            : (C) 2002/2016 by mean
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
#include "ADM_coreVideoEncoderInternal.h"
#include "ffvtenc.h"
#include "ffvtenc_desc.cpp"
#include "ADM_ffVTEnc.h"
extern bool ffVTEncConfigure(void);
extern ffvtenc VTEncSettings;

void resetConfigurationData()
{
    ffvtenc defaultConf = VT_ENC_CONF_DEFAULT;
    memcpy(&VTEncSettings, &defaultConf, sizeof(ffvtenc));
}

/**
 *   \fn ffVTEncProbe
 */
static bool ffVTEncProbe(void)
{
    return true;
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffVTEncoder);
#ifdef H265_ENCODER
ADM_DECLARE_VIDEO_ENCODER_MAIN_EX(
    "ffVTEncHEVC",
    "VideoToolbox HEVC",
    "VideoToolbox HEVC HW Encoder",
    ffVTEncConfigure,
    ADM_UI_ALL,
    1,0,0,
    ffvtenc_param,
    &VTEncSettings,NULL,NULL,
    ffVTEncProbe
);
#else
ADM_DECLARE_VIDEO_ENCODER_MAIN_EX(
    "ffVTEncH264",
    "VideoToolbox H.264",
    "VideoToolbox H.264 HW Encoder",
    ffVTEncConfigure,
    ADM_UI_ALL,
    1,0,0,
    ffvtenc_param,
    &VTEncSettings,NULL,NULL,
    ffVTEncProbe
);
#endif