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
#include "ADM_flv.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

ADM_DEMUXER_BEGIN( flvHeader, 50,
                    1,0,0,
                    "flv",
                    "flash demuxer plugin (c) Mean 2007/2007"
                );

/**
    \fn Probe
*/

extern "C"  uint32_t         probe(uint32_t magic, const char *fileName)
{
    if (fourCC::check (magic, (uint8_t *) "FLV\1"))
    {
	  printf (" [flvHeader] FLV file detected...\n");
	  return 100;
    }
    printf (" [flvHeader] Cannot open that\n");
    return 0;
}