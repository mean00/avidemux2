/***************************************************************************
     \file                     ADM_mxf.cpp
     \brief MXF demuxer
     \author mean, fixounet@free.fr (C) 2010
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_mxf.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

ADM_DEMUXER_BEGIN( mxfHeader, 50,
                    1,0,0,
                    "MXF",
                    "MXF demuxer plugin (c) Mean 2010"
                );

/**
    \fn Probe
*/

extern "C"  uint32_t         probe(uint32_t magic, const char *fileName)
{
uint32_t result=0;

    if(magic==0x342b0e06) 
    {

      printf("\n\t MXF file detected...\n");
      return 100;
    }
    
    printf (" [mxfHeader] Cannot open that (%x)\n",magic);
    return 0;
}
// EOF