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

#define ADM_NVENC_H264_IS_SUPPORTED 1
#define ADM_NVENC_HEVC_IS_SUPPORTED (1<<1)
#define ADM_NVENC_AV1_IS_SUPPORTED  (1<<2)

typedef struct {
    // The caller should set codecId to 1, 2 or 4 for H264, HEVC or AV1 respectively.
    // This bit will be cleared if the codec is not supported. In this case other
    // fields must be ignored. If more than one bit is set in codecId by the caller,
    // codecId will be set to zero and other fields must be considered invalid.
    uint32_t codecId;
    uint32_t rcMask;
    uint32_t maxWidth;
    uint32_t maxHeight;
    int maxBframes;
    uint32_t bframeRef; // if (bframeRef >> 2) hierarchical = true; /* H264 only */
    uint32_t lookahead; // if (lookahead & 1) level = (lookahead >> 1) & 0xF;
    bool lossless;
    bool temporalAq;
    bool weightedPred;
    bool highBitDepth;
} admNvencCaps;

bool ADM_getNvencCodecSupportFlags(admNvencCaps *caps);

// EOF
