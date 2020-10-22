/***************************************************************************
    \fn     utvideoEncPlugin
    \brief  Plugin for libavcodec Ut Video encoder

    copyright            : (C) 2002/2020 by mean
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
#include "ADM_utvideoEncoder.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "utvideo_encoder_desc.cpp"

extern bool utvideoConfigure(void);

void resetConfigurationData()
{
    utvideo_encoder defaultConf = UTVIDEO_CONF_DEFAULT;
    memcpy(&utconfig, &defaultConf, sizeof(utvideo_encoder));
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_utvideoEncoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN( "utvideo",
                                "Ut Video (ffmpeg)",
                                "Ut Video (c) 2020 Mean",
                                utvideoConfigure,
                                ADM_UI_ALL,
                                1,0,0,
                                utvideo_encoder_param, // conf template
                                &utconfig,NULL,NULL // conf var
);
