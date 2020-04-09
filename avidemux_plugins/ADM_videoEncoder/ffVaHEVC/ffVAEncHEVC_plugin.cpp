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
#include "ADM_ffVAEncHEVC.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_coreLibVA_encodingContext.h"
#include "ADM_coreLibVA_hevcEncoding.h"
#include "ffVAEnc_HEVC_desc.cpp"
extern bool ffVAEncHevcConfigure(void);
extern ffvaHEVC_encoder VaEncHevcSettings;

void resetConfigurationData()
{
	ffvaHEVC_encoder defaultConf = VAENC_HEVC_CONF_DEFAULT;

	memcpy(&VaEncHevcSettings, &defaultConf, sizeof(VaEncHevcSettings));
}

/**
 * \fn vaEncProbe
 */
static bool vaEncHevcProbe(void)
{
    VAProfile profile=vaGetHevcEncoderProfile()->profile;
    if(profile==VAProfileNone)
    {
        ADM_error("No HEVC encoding support\n");
        return false;
    }
    return true;
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffVAEncHEVC);
ADM_DECLARE_VIDEO_ENCODER_MAIN_EX("ffVAEncHEVC",
                               "Intel HEVC",
                               "Intel hw encoder",
                                ffVAEncHevcConfigure,
                                ADM_UI_ALL,
                                1,0,0,
                                ffvaHEVC_encoder_param, // conf template
                                &VaEncHevcSettings,NULL,NULL, // conf var
                                vaEncHevcProbe
);
