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

ADM_MUXER_BEGIN( muxerffTS,
                    1,0,0,
                    "ffTS",    // Internal name
                    "ffMpeg TS muxer plugin (c) Mean 2009",
                    "ffTS Muxer", // DIsplay name
                    ffTSConfigure,
                    ts_muxer_param, //template
                    &tsMuxerConfig //config
                );

