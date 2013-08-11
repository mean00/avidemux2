/***************************************************************************
                          \fn     ffFlv1Plugin
                          \brief  Plugin for flv1 encoder
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
#include "ADM_ffDv.h"
#include "ADM_coreVideoEncoderInternal.h"


void resetConfigurationData()
{
}
ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffDvEncoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("ffDv",
                               "DV (ffmpeg)",
                               "Simple ffmpeg based DV Encoder (c) 2013 Mean",
                                NULL, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                NULL, // conf template
                                NULL,NULL,NULL // conf var
);
