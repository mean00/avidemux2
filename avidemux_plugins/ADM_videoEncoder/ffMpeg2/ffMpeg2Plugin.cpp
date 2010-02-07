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
#include "ADM_ffMpeg2.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "../src/FFcodecSettings_desc.cpp"
extern bool         ffMpeg2Configure(void);
extern FFcodecSettings Mp2Settings;
ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_ffMpeg2Encoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("ffMpeg2",
                               "Mpeg2 (ff)",
                               "Simple ffmpeg based Mpeg2 Encoder (c) 2009 Mean",
                                ffMpeg2Configure, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                FFcodecSettings_param, // conf template
                                &Mp2Settings // conf var
);
