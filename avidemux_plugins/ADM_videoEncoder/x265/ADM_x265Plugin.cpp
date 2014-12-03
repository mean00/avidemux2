/***************************************************************************
                          \fn     x265Plugin
                          \brief  Plugin for x265 dummy encoder
                             -------------------

    copyright            : (C) 2002/2014 by mean/gruntster
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
#include "ADM_x265.h"
#include "ADM_coreVideoEncoderInternal.h"
extern "C"
{
#include "x265_settings_desc.cpp"
}
extern bool         x265Configure(void);
extern x265_settings x265Settings;
extern bool x265LoadProfile(const char *profile);
void resetConfigurationData()
{
	x265_settings defaultConf = X265_DEFAULT_CONF;

	memcpy(&x265Settings, &defaultConf, sizeof(x265_settings));
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(x265Encoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("x265",
                               "HEVC (x265)",
                               "x265 based HEVC Encoder (c) 2014 Mean/Gruntster",
                                x265Configure, // No configuration
                                ADM_UI_TYPE_BUILD,
                                1,0,0,
                                x265_settings_param, // conf template
                                &x265Settings, // conf var
                               x265LoadProfile, // setProfile
                               NULL  // getProfile
);

