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
#include "ADM_ffv1Encoder.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ffv1_encoder_desc.cpp"

extern bool ffv1Configure(void);

void resetConfigurationData()
{
    ffv1_encoder defaultConf = FFV1_CONF_DEFAULT;
    memcpy(&ffv1config, &defaultConf, sizeof(ffv1_encoder));
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffv1Encoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("FFV1",
                               "FFV1",
                               "FFV1 (c) 2021 szlldm",
                                ffv1Configure,
                                ADM_UI_ALL,
                                1,0,0,
                                ffv1_encoder_param, // conf template
                                &ffv1config,NULL,NULL // conf var
);
