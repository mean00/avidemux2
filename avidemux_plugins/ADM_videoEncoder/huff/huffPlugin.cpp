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
#include "ADM_huffEncoder.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "huff_encoder_desc.cpp"

extern bool         huffConfigure(void);

void resetConfigurationData()
{
	huff_encoder defaultConf = HUFF_CONF_DEFAULT;

	memcpy(&huffType, &defaultConf, sizeof(huff_encoder));
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_huffEncoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("HUFFYUV",
                               "(FF)HuffYUV",
                               "FF Huffyuv (c) 2009 Mean",
                                huffConfigure, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                huff_encoder_param, // conf template
                                &huffType,NULL,NULL // conf var
);
