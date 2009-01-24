/***************************************************************************
                          ADM_codecwav.cpp  -  description
                             -------------------
    begin                : Fri May 31 2002
    copyright            : (C) 2002 by mean
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sstream>
#include "ADM_assert.h"
#include <math.h>

#include "config.h"
#include "avifmt.h"
#include "avifmt2.h"
#include "fourcc.h"
//#include "ADM_audio/aviaudio.hxx"
#include "ADM_audiocodec/ADM_audiocodec.h"

ADM_AudiocodecWav::ADM_AudiocodecWav( uint32_t fourcc ) : ADM_Audiocodec(fourcc)
{

}
ADM_AudiocodecWav::~ADM_AudiocodecWav()
{

}

uint8_t ADM_AudiocodecWav::beginDecompress()
{
         return 1;
}
uint8_t ADM_AudiocodecWav::endDecompress()
{
       return 1;
}

uint8_t ADM_AudiocodecWav::isCompressed( void )
{
 	return 0;
}

uint8_t ADM_AudiocodecWav::run(uint8_t * inptr, uint32_t nbIn, float * outptr, uint32_t * nbOut)
{
	int16_t *in = (int16_t *) inptr;

	*nbOut=nbIn / 2;
	for (int i = 0; i < *nbOut; i++) {
		*(outptr++) = (float)*in / 32768;
		in++;
	}

	return 1;
}


