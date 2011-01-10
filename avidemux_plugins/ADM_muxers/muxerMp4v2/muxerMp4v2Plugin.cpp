/***************************************************************************
    copyright            : (C) 2007 by mean
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
#include "ADM_default.h"
#include "ADM_muxerInternal.h"
#include "fourcc.h"
#include "muxerMp4v2.h"

bool mp4v2Configure(void)
{
    return true;
}
ADM_MUXER_BEGIN( muxerMp4v2,
                    1,0,0,
                    "MP4V2",    // Internal name
                    "MP4V2 muxer plugin (c) Mean 2011",
                    "MP4 Muxer", // DIsplay name
                    mp4v2Configure,
                    NULL, //template
                    NULL
                );


