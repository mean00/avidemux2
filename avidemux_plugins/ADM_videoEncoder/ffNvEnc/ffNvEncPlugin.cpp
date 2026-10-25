/***************************************************************************
                          \fn     nvEnc
                          \brief  Plugin for nvEnc lav encoder
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
#include "ADM_ffNvEnc.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_coreNvenc.h"
#include "ffnvenc_desc.cpp"
extern bool            ffNvEncConfigure(void);
extern ffnvenc_encoder NvEncSettings;

void resetConfigurationData()
{
	ffnvenc_encoder defaultConf = NVENC_CONF_DEFAULT;

	memcpy(&NvEncSettings, &defaultConf, sizeof(ffnvenc_encoder));
}

/**
 * \fn nvEncProbe
 */
static bool nvEncProbe(void)
{
    admNvencCaps caps;
    caps.codecId =
#ifdef H265_ENCODER
        ADM_NVENC_HEVC_IS_SUPPORTED;
#elif defined(AV1_ENCODER)
        ADM_NVENC_AV1_IS_SUPPORTED;
#else
        ADM_NVENC_H264_IS_SUPPORTED;
#endif

    if (false == ADM_getNvencCodecSupportFlags(&caps))
        return false;

    return !!caps.codecId;
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffNvEncEncoder);
#ifdef H265_ENCODER
ADM_DECLARE_VIDEO_ENCODER_MAIN_EX("ffNvEncHEVC",
                               "Nvidia HEVC",
                               "Nvidia hw encoder",
                                ffNvEncConfigure, // configuration
                                ADM_UI_ALL,
                                1,0,0,
                                ffnvenc_encoder_param, // conf template
                                &NvEncSettings,NULL,NULL, // conf var
                                nvEncProbe
);
#elif defined(AV1_ENCODER)
ADM_DECLARE_VIDEO_ENCODER_MAIN_EX("ffNvEncAV1",
                               "Nvidia AV1",
                               "Nvidia hw encoder",
                                ffNvEncConfigure, // configuration
                                ADM_UI_ALL,
                                1,0,0,
                                ffnvenc_encoder_param, // conf template
                                &NvEncSettings,NULL,NULL, // conf var
                                nvEncProbe
);
#else
ADM_DECLARE_VIDEO_ENCODER_MAIN_EX("ffNvEncH264",
                               "Nvidia H264",
                               "Nvidia hw encoder",
                                ffNvEncConfigure, // configuration
                                ADM_UI_ALL,
                                1,0,0,
                                ffnvenc_encoder_param, // conf template
                                &NvEncSettings,NULL,NULL, // conf var
                                nvEncProbe
);
#endif
