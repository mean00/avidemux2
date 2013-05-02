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
#include "ADM_ffMsMp4.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "../src/FFcodecSettings_desc.cpp"
extern bool         ffMsMp4Configure(void);
extern FFcodecSettings MsMp4Settings;

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffMsMp4Encoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("ffMsMp4",
                               "MsMpeg4(Divx3)",
                               "Simple ffmpeg based ms mpeg4 Encoder (c) 2009 Mean",
                                ffMsMp4Configure, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                 FFcodecSettings_param, // conf template
                                &MsMp4Settings,NULL,NULL // conf var
);
