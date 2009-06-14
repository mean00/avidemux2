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
#include "muxerMP4.h"

#include "fourcc.h"
 bool mp4Configure(void);

ADM_MUXER_BEGIN( muxerMP4,
                    1,0,0,
                    "MP4",    // Internal name
                    "MP4 muxer plugin (c) Mean 2008",
                    "MP4 Muxer", // DIsplay name
                    mp4Configure,
                    &muxerConfig,
                    sizeof(muxerConfig)
                );

