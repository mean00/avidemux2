/***************************************************************************
                          avifmt2.h  -  description
                             -------------------
    begin                : Thu Nov 1 2001
    copyright            : (C) 2001 by mean
    email                : fixounet@free.fr

	This part of header is ripped directly from wine project
  See http://winehq.com to see more of their great work

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __AVIFMT2__
#define __AVIFMT2__

#include "ADM_bitmap.h"

void printBih(ADM_BITMAPINFOHEADER *bi);



typedef struct _avistdindex_chunk 
{
        uint16_t 	wLongsPerEntry;		// must be sizeof(aIndex[0])/sizeof(DWORD)
        uint8_t 	bIndexSubType;			// must be 0
        uint8_t 	bIndexType;			// must be AVI_INDEX_OF_CHUNKS
        uint32_t 	nEntriesInUse;		//
        uint32_t 	dwChunkId;			// �##dc� or �##db� or �##wb� etc..
        uint32_t        qw1,qw2;			// MN.
        uint32_t 	dwReserved3;				// must be 0
	
} AVISTDINDEX, * PAVISTDINDEX;



#define AVI_KEY_FRAME   0x10
#define AVI_B_FRAME	0x4000	 // hopefully it is not used..

#include "ADM_coreAudio.h"

void Endian_AviMainHeader(MainAVIHeader *m);
void Endian_BitMapInfo( ADM_BITMAPINFOHEADER *b);
void Endian_AviStreamHeader(AVIStreamHeader *s);
void Endian_WavHeader(WAVHeader *w);

#endif
//EOF

