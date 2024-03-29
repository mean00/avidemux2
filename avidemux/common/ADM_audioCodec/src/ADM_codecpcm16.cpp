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
#include "ADM_default.h"
#include "ADM_audiocodec.h"

ADM_AudiocodecWav::ADM_AudiocodecWav( uint32_t fourcc,const WAVHeader &info ) : ADM_Audiocodec(fourcc,info)
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

    ADM_info("Creating not swapped wav decoder (PCM)\n");
}
ADM_AudiocodecWav::~ADM_AudiocodecWav()
{

}

uint8_t ADM_AudiocodecWav::isCompressed( void )
{
 	return 0;
}

uint8_t ADM_AudiocodecWav::run(uint8_t * inptr, uint32_t nbIn, float * outptr, uint32_t * nbOut)
{
    uint32_t i,j,sampleSize=0;
    switch(wavHeader.bitspersample)
    {
        case 16: sampleSize=2;break;
        case 24: sampleSize=3;break;
        case 32: sampleSize=4;break;
        case 64: sampleSize=8;break;
        default: break;
    }
    ADM_assert(sampleSize);
    if (nbIn < sampleSize)
        return 1;

    if (nbIn % sampleSize)
    {
        ADM_error("Error: nbIn (%i) not multiple of bytes per sample in lpcm", nbIn);
        abort();
    }

    *nbOut = nbIn / sampleSize;
    for (i = 0; i < *nbOut; i++)
    {
        uint64_t sample = 0;
        for (j = 0; j < sampleSize; j++)
            sample |= ((uint64_t)inptr[j]) << (64 - wavHeader.bitspersample + 8 * j);
        *(outptr++) = (float)(int64_t)sample / (1LL << 63);
        inptr += sampleSize;
    }

    return 1;
}


