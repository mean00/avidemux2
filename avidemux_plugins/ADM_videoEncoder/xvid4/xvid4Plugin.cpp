/***************************************************************************
                          \fn     xvid4Plugin
                          \brief  Plugin for xvid4 dummy encoder
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
#include "ADM_xvid4.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "xvid4_encoder_desc.cpp"
extern bool         xvid4Configure(void);
extern xvid4_encoder xvid4Settings;

void resetConfigurationData()
{
	xvid4_encoder defaultConf = XVID_DEFAULT_CONF;

	memcpy(&xvid4Settings, &defaultConf, sizeof(xvid4_encoder));
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(xvid4Encoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("xvid4",
                               "Mpeg4 ASP (xvid4)",
                               "Xvid4 based mpeg4 Encoder (c) 2010 Mean/Gruntster",
                                xvid4Configure, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                xvid4_encoder_param, // conf template
                                &xvid4Settings,NULL,NULL // conf var
);
