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
#include "ADM_asf.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

ADM_DEMUXER_BEGIN( asfHeader,
                    1,0,0,
                    "asf",
                    "asf/wmv demuxer plugin (c) Mean 2007/2009"
                );

/**
    \fn Probe
*/

extern "C"  uint32_t         probe(uint32_t magic, const char *fileName)
{
        const uint8_t signature[4]={ 0x30,0x26,0xb2,0x75}; //0x75,0xb2,0x26,0x30}; 

    if (fourCC::check (magic, signature))
    {
	  printf (" [asfHeader] FLV file detected...\n");
	  return 100;
    }
    printf (" [asfHeader] Cannot open that\n");
    return 0;
}
