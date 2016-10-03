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
#include "ADM_libvaEncoder.h"
#include "ADM_coreVideoEncoderInternal.h"

void resetConfigurationData() {}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_libvaEncoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("LibVaEncoder (HW)",
                               "Mpeg4 AVC (VA/HW)",
                               "Simple Libva Encoder (c) 2013 Mean",
                                NULL, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                NULL, // conf template
                                NULL, // conf var
                                NULL,NULL
);

