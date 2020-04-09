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
#include "ADM_ffVAEncH264.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_coreLibVA_encodingContext.h"
#include "ADM_coreLibVA_h264Encoding.h"
#include "ffVAEnc_H264_desc.cpp"
extern bool            ffVAEncConfigure(void);
extern ffvaenc_encoder VaEncSettings;

void resetConfigurationData()
{
	ffvaenc_encoder defaultConf = VAENC_CONF_DEFAULT;

	memcpy(&VaEncSettings, &defaultConf, sizeof(VaEncSettings));
}

/**
 * \fn vaEncProbe
 */
static bool vaEncProbe(void)
{
    VAProfile profile=vaGetH264EncoderProfile()->profile;
    if(profile==VAProfileNone)
    {
        ADM_error("No H264 encoding support\n");
        return false;
    }
    return true;
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffVAEncH264Encoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN_EX("ffVAEncH264",
                               "Intel H264",
                               "Intel hw encoder",
                                ffVAEncConfigure,
                                ADM_UI_ALL,
                                1,0,0,
                                ffvaenc_encoder_param, // conf template
                                &VaEncSettings,NULL,NULL, // conf var
                                vaEncProbe
);
