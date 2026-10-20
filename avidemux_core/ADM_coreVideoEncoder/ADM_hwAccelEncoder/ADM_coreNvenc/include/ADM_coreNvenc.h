/***************************************************************************
    \fn ADM_coreNvenc.h
    \brief Probe codecs supported by NVENC interface and hw

    copyright            : (C) 2018 by mean
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
#pragma once
#include "ADM_default.h"
#include "ADM_coreVideoEncoder6_export.h"

#define ADM_NVENC_H264_IS_SUPPORTED 1
#define ADM_NVENC_HEVC_IS_SUPPORTED (1<<1)
#define ADM_NVENC_AV1_IS_SUPPORTED  (1<<2)

ADM_COREVIDEOENCODER6_EXPORT bool ADM_getNvencCodecSupportFlags(uint32_t *flags);

// EOF
