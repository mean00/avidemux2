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
#include "ADM_wtv.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

ADM_DEMUXER_BEGIN( wtvHeader, 50,
                    1,0,0,
                    "wtv",
                    "wtv DUMMY demuxer plugin (c) Mean 2010"
                );

/**
    \fn Probe
*/

extern "C"  uint32_t         probe(uint32_t magic, const char *fileName)
{
    const uint8_t signature[16]= {0xB7,0xD8,0x00,0x20,0x37,0x49,0xDA,0x11,0xA6,0x4E,0x00,0x07,0xE9,0x5E,0xAD,0x8D};

    FILE *f=ADM_fopen(fileName,"rb");
    if(!f)
    {
            ADM_error("Cannot open %s\n",fileName);
            return 0;
    }
    uint8_t buffer[16];
    fread(buffer,16,1,f);
    fclose(f);
    if(!memcmp(buffer,signature,16))
    {
            ADM_info("WTV signature found\n");
            return 100;
    }

    ADM_info (" [wtv] Cannot open that\n");
    return 0;
}
