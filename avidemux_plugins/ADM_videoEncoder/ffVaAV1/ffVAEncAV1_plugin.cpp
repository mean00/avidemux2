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
#include "ADM_ffVAEncAV1.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_coreLibVA_encodingContext.h"
#include "ADM_coreLibVA_av1Encoding.h"
#include "ffVAEnc_AV1_desc.cpp"
extern bool ffVAEncAV1Configure(void);
extern ffvaAV1_encoder VaEncAV1Settings;

void resetConfigurationData()
{
	ffvaAV1_encoder defaultConf = VAENC_AV1_CONF_DEFAULT;

	memcpy(&VaEncAV1Settings, &defaultConf, sizeof(VaEncAV1Settings));
}

/**
 * \fn vaEncProbe
 */
static bool vaEncAV1Probe(void)
{
    VAProfile profile=vaGetAV1EncoderProfile()->profile;
    if(profile==VAProfileNone)
    {
        ADM_error("No AV1 encoding support\n");
        return false;
    }
    return true;
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffVAEncAV1);
ADM_DECLARE_VIDEO_ENCODER_MAIN_EX("ffVAEncAV1",
                               "Intel AV1",
                               "Intel hw encoder",
                                ffVAEncAV1Configure,
                                ADM_UI_ALL,
                                1,0,0,
                                ffvaAV1_encoder_param, // conf template
                                &VaEncAV1Settings,NULL,NULL, // conf var
                                vaEncAV1Probe
);
