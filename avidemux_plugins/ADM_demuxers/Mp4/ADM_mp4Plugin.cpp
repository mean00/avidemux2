/***************************************************************************
        \file ADM_mp4Plugin.cpp
    copyright            : (C) 2008 by mean
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
#include "ADM_mp4.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

ADM_DEMUXER_BEGIN( MP4Header,
                    1,0,0,
                    "mp4",
                    "Mp4/Mov/3GP demuxer plugin (c) Mean 2007/2008"
                );

/**
    \fn Probe
*/

extern "C"  uint32_t         probe(uint32_t magic, const char *fileName)
{
uint8_t head[8];
    FILE *f=fopen(fileName,"r");
    if(!f) return 0;
    fread(head,8,1,f);
    fclose(f);
    uint8_t *z=&(head[4]);
 if(fourCC::check(z,(uint8_t *)"ftyp") ||
    fourCC::check(z,(uint8_t *)"pnot") ||
	fourCC::check(z,(uint8_t *)"mdat") ||
	fourCC::check(z,(uint8_t *)"moov") ||
	fourCC::check(z,(uint8_t *)"wide") ||
        fourCC::check(magic,(uint8_t *)"skip"))
    {
            printf (" [MP4]MP4/MOV/3GP file detected...\n");
	        return 100;

    }
    printf (" [MP4] Cannot open that...\n");
    return 0;
}
