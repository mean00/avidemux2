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
#include "muxerffTS.h"
#include "ts_muxer_desc.cpp"
#include "fourcc.h"
 bool ffTSConfigure(void);

static const char *getTsExt(void)
{
    if (getenv("ADM_TS_MUXER_EXT_ALL_CAPS"))
        return tsMuxerConfig.m2TsMode ? "M2TS" : "TS";
    return tsMuxerConfig.m2TsMode ? "m2ts" : "ts";
}

ADM_MUXER_DYN_EXT(
    getTsExt,
    muxerffTS,
    1,0,1,
    "ffTS", // Internal name
    "ffMpeg TS muxer plugin (c) Mean 2009",
    "Mpeg TS Muxer (ff)", // Display name
    ffTSConfigure,
    ts_muxer_param, // template
    &tsMuxerConfig, // config
    sizeof(ts_muxer)
)

