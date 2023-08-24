/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_av1AomEnc.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "av1aom_encoder.h"
#include "av1aom_encoder_desc.cpp"

extern bool av1AomEncoderConfigure(void);
extern av1aom_encoder encoderSettings;

void resetConfigurationData(void)
{
    av1aom_encoder conf = AV1_DEFAULT_CONF;
    memcpy(&encoderSettings, &conf, sizeof(encoderSettings));
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(av1AomEncoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN (
    "av1",
    "AV1 (libaom)",
    "libaom-based AV1 Encoder",
    av1AomEncoderConfigure, // configuration dialog
    ADM_UI_ALL,
    1,0,0,
    av1aom_encoder_param, // conf template
    &encoderSettings, // conf var
    NULL, // setProfile
    NULL  // getProfile
);

// EOF

