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
#include "ADM_ffFlv1.h"
#include "ADM_coreVideoEncoderInternal.h"

extern bool         ffFlv1Configure(void);

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffFlv1Encoder);
ADM_DECLARE_VIDEO_ENCODER_NO_CONFIG();
ADM_DECLARE_VIDEO_ENCODER_MAIN("ffFlv1",
                               "FLV1(flash)",
                               "Simple ffmpeg based Flv1 Encoder (c) 2009 Mean",
                                ffFlv1Configure, // No configuration
                                ADM_UI_ALL,
                                1,0,0);
