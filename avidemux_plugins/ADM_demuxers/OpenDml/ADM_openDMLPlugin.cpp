/***************************************************************************
        \file ADM_openDMLPlugin.cpp
    copyright            : (C) 2008 by mean
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
#include "ADM_openDML.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

ADM_DEMUXER_BEGIN( OpenDMLHeader, 50,
                    1,0,0,
                    "openDml",
                    "AVI/OpenDML demuxer plugin (c) Mean 2007/2007"
                );

/**
    \fn Probe
*/

extern "C"  uint32_t         probe(uint32_t magic, const char *fileName)
{
    if (fourCC::check (magic, (uint8_t *) "RIFF"))
    {
        // Make sure it is really an avi...
          FILE *f=ADM_fopen(fileName,"r");
          if(!f) return 0;
          uint8_t buf[12];
          fread(buf,12,1,f);
          fclose(f);
          if(fourCC::check(buf+8,(uint8_t *)"AVI "))
          {
	        printf (" [openDML] AVI/OpenDML file detected...\n");
	        return 100;
          }
    }
    printf (" [openDML] Cannot open that\n");
    return 0;
}
