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
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_jpegEncoder.h"

#include "jpeg_encoder_desc.cpp"

extern bool         jpegConfigure(void);

void resetConfigurationData()
{
	jpeg_encoder defaultConf = JPEG_CONF_DEFAULT;

	memcpy(&jpegConf, &defaultConf, sizeof(jpeg_encoder));
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_jpegEncoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("Mjpeg",
                               "Mjpeg Encoder",
                               "Simple jpeg Encoder (c) 2009 Mean",
                                jpegConfigure, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                jpeg_encoder_param, // conf template
                                &jpegConf,NULL,NULL // conf var
);
