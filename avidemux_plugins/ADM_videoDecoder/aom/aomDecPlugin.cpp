/***************************************************************************
    \fn     aomDecPlugin
    \brief  Video decoder plugin for libaom
    \author eumagga0x2a based on vpx decoder by mean fixounet@free.fr (C) 2010
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
#include "ADM_coreVideoDecoderInternal.h"
#include "ADM_aomDec.h"
static uint32_t fccs[]={MKFCC('a','v','0','1'),0};
ADM_DECLARE_VIDEO_DECODER_PREAMBLE(decoderAom);
ADM_DECLARE_VIDEO_DECODER_MAIN("aom",
                               "AV1",
                               "Decoder using libaom (c) mean 2010 / eumagga0x2a 2019",
                                fccs, // No configuration
                                NULL,
                                1,0,0
);
