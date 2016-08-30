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
#include "ADM_pics.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

ADM_DEMUXER_BEGIN( picHeader, 50,
                    1,0,0,
                    "picture",
                    "Picture demuxer plugin (c) Mean 2007/2008"
                );

/**
    \fn Probe
*/

extern "C"  uint32_t         probe(uint32_t magic, const char *fileName)
{
uint32_t result=0;

    if(magic==0x474e5089) 
    {

      ADM_info (" PNG file detected...\n");
      return 100;
    }
    if((magic&0xffff)==0xd8ff) 
    {

      ADM_info ("  JPG file detected...\n");
      return 100;
    }
    if((magic & 0xffff)==0x4d42) 
    {

      ADM_info ("  BMP file detected...\n");
      return 100;
    }
    
    ADM_info (" [picHeader] Cannot open that\n");
    return 0;
}
