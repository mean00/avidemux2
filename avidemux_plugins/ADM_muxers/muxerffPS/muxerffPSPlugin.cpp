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
#include "ADM_includeFfmpeg.h"
#include "ADM_default.h"
#include "ADM_muxerInternal.h"
#include "muxerffPS.h"
#include "ps_muxer_desc.cpp"
#include "fourcc.h"
 bool ffPSConfigure(void);

ADM_MUXER_BEGIN( muxerffPS,
                    1,0,0,
                    "ffPS",    // Internal name
                    "ffMpeg PS muxer plugin (c) Mean 2009",
                    "ffPS Muxer", // DIsplay name
                    ffPSConfigure,
                    ps_muxer_param, //template
                    &psMuxerConfig //config
                );

