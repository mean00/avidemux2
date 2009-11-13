/***************************************************************************
                          \fn     yv12Plugin
                          \brief  Plugin for YV12 dummy encoder
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
#include "ADM_pngEncoder.h"
#include "ADM_coreVideoEncoderInternal.h"


ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_pngEncoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("PNG",
                               "PNG Encoder",
                               "Simple PNG Encoder (c) 2009 Mean",
                                NULL, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                NULL, // conf template
                                NULL // conf var
);