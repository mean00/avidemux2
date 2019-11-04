/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_vp9.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "vp9_encoder.h"
#include "vp9_encoder_desc.cpp"

extern bool vp9EncoderConfigure(void);
extern vp9_encoder vp9Settings;

void resetConfigurationData()
{
    vp9_encoder conf = VP9_DEFAULT_CONF;
    memcpy(&vp9Settings, &conf, sizeof(vp9_encoder));
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(vp9Encoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("vp9",
                               "VP9 (libvpx)",
                               "libvpx based VP9 Encoder",
                                vp9EncoderConfigure, // configuration dialog
                                ADM_UI_ALL,
                                1,0,0,
                                vp9_encoder_param, // conf template
                                &vp9Settings, // conf var
                                NULL, // setProfile
                                NULL  // getProfile
);

// EOF

