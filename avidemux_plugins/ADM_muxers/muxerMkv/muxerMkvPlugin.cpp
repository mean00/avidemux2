/***************************************************************************
    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr

      See lavformat/flv[dec/env].c for detail
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
#include "ADM_muxerInternal.h"
#include "muxerMkv.h"
#include "mkv_muxer_desc.cpp"

#ifdef MUXER_IS_WEBM
extern bool WebmConfigure(void);
ADM_MUXER_BEGIN(
        "webm",muxerWebm,
        1,0,0,
        "WEBM",          // Internal name
        "WebM muxer plugin (c) Mean 2009",
        "WebM Muxer",    // Display name
        WebmConfigure,   // configure function
        mkv_muxer_param, // Template
        &muxerConfig,    // conf
        sizeof(mkv_muxer));
#else
extern bool mkvConfigure(void);
ADM_MUXER_BEGIN(
        "mkv",muxerMkv,
        1,0,0,
        "MKV",           // Internal name
        "Matroska muxer plugin (c) Mean 2009",
        "MKV Muxer",     // Display name
        mkvConfigure,    // configure function
        mkv_muxer_param, // Template
        &muxerConfig,    // conf
        sizeof(mkv_muxer));
#endif
