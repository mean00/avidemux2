
/***************************************************************************
                        Probe for a stream                                              
                             
    
    copyright            : (C) 2005 by mean
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

#ifndef DMX_PROBE
#define DMX_PROBE
#define BUFFER_SIZE (10*1024)
uint8_t dmx_probe(const char *file, DMX_TYPE  *type, uint32_t *nbTracks,MPEG_TRACK **tracks);
// For test
uint8_t runProbe(char *f);
#endif
