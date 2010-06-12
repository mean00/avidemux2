/***************************************************************************
                          \fn     vpxPlugin
                          \brief  Video decoder Plugin for libvpx
                          \author mean fixounet@free.fr (C) 2010
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
#include "ADM_vpx.h"
static uint32_t fccs[]={MKFCC('V','P','8',' '),0};
ADM_DECLARE_VIDEO_DECODER_PREAMBLE(decoderVPX);
ADM_DECLARE_VIDEO_DECODER_MAIN("vpx",
                               "VP8/WebmM",
                               "Decoder using libvpx (c) mean 2010",
                                fccs, // No configuration
                                NULL,
                                1,0,0
);