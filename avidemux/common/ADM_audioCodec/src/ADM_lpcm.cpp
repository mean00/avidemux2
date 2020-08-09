/***************************************************************************
                          ADM_lpcm .cpp  -  description
                             -------------------

	Handle DVD LPCM, i.e. swapped PC PCM
    begin                : Fri May 5 2003
    copyright            : (C) 2003 by mean
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
#include "ADM_audiocodec.h"

ADM_AudiocodecWavSwapped::ADM_AudiocodecWavSwapped( uint32_t fourcc,const WAVHeader &info ) : ADM_Audiocodec(fourcc,info)
{
    CHANNEL_TYPE *p_ch_type = channelMapping;

#define MAPME(ch) *(p_ch_type++)=ADM_CH_##ch;
    MAPME(FRONT_LEFT)
    MAPME(FRONT_RIGHT)
    MAPME(FRONT_CENTER)
    MAPME(LFE)
    MAPME(REAR_LEFT)
    MAPME(REAR_RIGHT)
    MAPME(SIDE_LEFT)
    MAPME(SIDE_RIGHT)

    ADM_info("Creating swapped wav decoder (LPCM)\n");
}

ADM_AudiocodecWavSwapped::~ADM_AudiocodecWavSwapped()
{

}

uint8_t ADM_AudiocodecWavSwapped::isCompressed( void )
{
 	return 1;
}

uint8_t ADM_AudiocodecWavSwapped::run(uint8_t * inptr, uint32_t nbIn, float *outptr, uint32_t * nbOut)
{
	if (nbIn < 2)
		return 1;

	if (nbIn&1) {
		ADM_error("Error: nbIn (%i) odd in lpcm", nbIn);
		abort();
	}

	int16_t sample;
	*nbOut=nbIn / 2;
	for (int i = 0; i < *nbOut; i++) {
		sample = (inptr[0] << 8) | inptr[1];
		*(outptr++) = (float)sample / 32768;
		inptr += 2;
	}

	return 1;
}

