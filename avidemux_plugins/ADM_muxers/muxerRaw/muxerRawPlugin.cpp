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
#include "muxerRaw.h"

#include "fourcc.h"
static bool rawConfigure(void) {return true;};

ADM_MUXER_BEGIN( "raw",muxerRaw,
                    1,0,0,
                    "RAW",    // Internal name
                    "RAW muxer plugin (c) Mean 2008",
                    "Video Only", // DIsplay name
                    rawConfigure,
                    NULL,0
                    
                );

