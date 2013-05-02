/***************************************************************************
                          \fn     nullEncoder
                          \brief  Plugin for  dummy encoder
                             -------------------

    copyright            : (C) 2002/2010 by mean
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
#include "nullEncoder.h"
#include "ADM_coreVideoEncoderInternal.h"

extern bool         nullConfigure(void);

void resetConfigurationData() {}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_nullEncoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("null",
                               "null",
                               "Null Encoder (c) 2010 Mean",
                                NULL, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                NULL, // conf template
                                NULL,NULL,NULL // conf var
);
