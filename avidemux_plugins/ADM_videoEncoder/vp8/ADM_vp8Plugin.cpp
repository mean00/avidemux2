/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_vp8.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "vp8_encoder.h"
#include "vp8_encoder_desc.cpp"

extern bool vp8EncoderConfigure(void);
extern vp8_encoder vp8Settings;

void resetConfigurationData()
{
    vp8_encoder conf = VP8_DEFAULT_CONF;
    memcpy(&vp8Settings, &conf, sizeof(vp8_encoder));
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(vp8Encoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("vp8",
                               "VP8 (libvpx)",
                               "libvpx based VP8 Encoder",
                                vp8EncoderConfigure, // configuration dialog
                                ADM_UI_ALL,
                                1,0,0,
                                vp8_encoder_param, // conf template
                                &vp8Settings, // conf var
                                NULL, // setProfile
                                NULL  // getProfile
);

// EOF

