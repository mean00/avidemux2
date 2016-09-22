/***************************************************************************
                          \fn     jpegPlugin
                          \brief  Plugin for jpeg dummy encoder
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

#include "ADM_default.h"
#include "ADM_ffNvEnc.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ffnvenc_desc.cpp"
extern bool            ffNvEncConfigure(void);
extern ffnvenc_encoder NvEncSettings;

void resetConfigurationData()
{
	ffnvenc_encoder defaultConf = NVENC_CONF_DEFAULT;

	memcpy(&NvEncSettings, &defaultConf, sizeof(FFcodecSettings));
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffNvEncEncoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("ffNvEnc",
                               "Nvidia H264 Encoder (lav)",
                               "Nvidia hw encoder",
                                ffNvEncConfigure, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                ffnvenc_encoder_param, // conf template
                                &NvEncSettings,NULL,NULL // conf var
);
