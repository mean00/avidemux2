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
#include "muxerFlv.h"

#include "fourcc.h"
 bool flvConfigure(void);

ADM_MUXER_BEGIN( muxerFlv,
                    1,0,0,
                    "FLV",    // Internal name
                    "FLV muxer plugin (c) Mean 2008",
                    "FLV Muxer", // DIsplay name
                    flvConfigure,
                    &muxerConfig,
                    sizeof(muxerConfig)
                );

