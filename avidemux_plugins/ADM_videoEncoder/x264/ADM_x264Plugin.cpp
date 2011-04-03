/***************************************************************************
                          \fn     x264Plugin
                          \brief  Plugin for x264 dummy encoder
                             -------------------
    
    copyright            : (C) 2002/2009 by mean/gruntster
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
#include "ADM_x264.h"
#include "ADM_coreVideoEncoderInternal.h"
extern "C"
{
#include "x264_encoder_desc.cpp"
}
extern bool         x264Configure(void);
extern x264_encoder x264Settings;
ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(x264Encoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("x264",
                               "Mpeg4 AVC (x264)",
                               "x264 based mpeg4 AVC Encoder (c) 2010 Mean/Gruntster",
                                x264Configure, // No configuration
                                ADM_UI_TYPE_BUILD,
                                1,0,0,
                                x264_encoder_param, // conf template
                                &x264Settings // conf var
);
