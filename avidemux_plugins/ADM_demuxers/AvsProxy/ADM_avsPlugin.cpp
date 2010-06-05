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
#include "ADM_avsproxy.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

ADM_DEMUXER_BEGIN( avsHeader, 50,
                    1,0,0,
                    "avs",
                    "avsProxy demuxer plugin (c) Mean 2007/2010"
                );

/**
    \fn Probe
*/

extern "C"  uint32_t         probe(uint32_t magic, const char *fileName)
{
       
    if (!strcmp(fileName,AVS_PROXY_DUMMY_FILE))
    {
	  printf (" [avsProxy] AvsProxy pseudo file detected...\n");
	  return 100;
    }
    printf (" [avsProxy] Cannot open that\n");
    return 0;
}
