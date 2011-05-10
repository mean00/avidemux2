/***************************************************************************
                     ADM_ad_opencore.cpp  -  description
                     -----------------------------------

    begin                : Tue Jul 14 2009
    copyright            : (C) 2009 by mean/Gruntster
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
#include "ADM_ad_plugin.h"

extern "C" {
#include <opencore-amrwb/dec_if.h>
#include <opencore-amrwb/if_rom.h>
};

#define ADM_AMR_BUFFER (16 * 1024) // 16 kB internal

class ADM_AudiocodecOpencoreAmrWb : public ADM_Audiocodec
{
protected:
	void *state;
	uint8_t _buffer[ADM_AMR_BUFFER];
	uint32_t _head, _tail;

public:
	ADM_AudiocodecOpencoreAmrWb(uint32_t fourcc, WAVHeader *info,uint32_t extraLength,uint8_t *extraDatab);
	~ADM_AudiocodecOpencoreAmrWb();
	uint8_t beginDecompress(void);
	uint8_t endDecompress(void);
	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
	uint8_t isCompressed(void) { return 1; }
	uint8_t isDecompressable(void) { return 1; }
};

static  ad_supportedFormat Formats[]={
        {WAV_AMRWB,AD_MEDIUM_QUAL},
};
DECLARE_AUDIO_DECODER(ADM_AudiocodecOpencoreAmrWb,	// Class
1,0,0, 												// Major, minor,patch 
Formats, 											// Supported formats
"opencore-amrwb decoder plugin for Avidemux (c) Mean/Gruntster\n"); 	// Desc

uint8_t ADM_AudiocodecOpencoreAmrWb::beginDecompress( void ) 
{
	_tail = _head = 0;

	return 1;
};

uint8_t ADM_AudiocodecOpencoreAmrWb::endDecompress( void ) 
{
	_tail = _head = 0;
	return 1;
};

ADM_AudiocodecOpencoreAmrWb::ADM_AudiocodecOpencoreAmrWb(
	uint32_t fourcc, WAVHeader *info, uint32_t extraLength, uint8_t *extraData) : ADM_Audiocodec(fourcc,*info)
{
	state = D_IF_init();
	

	info->channels = 1;
}

ADM_AudiocodecOpencoreAmrWb::~ADM_AudiocodecOpencoreAmrWb( )
{
	if (state)
	{
		D_IF_exit(state);
		state = NULL;
	}
}

uint8_t ADM_AudiocodecOpencoreAmrWb::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
#define SAMPLE_PER_FRAME 320

	static const short block_size[16] = { 18, 24, 33, 37, 41, 47, 51, 59, 61, 6, 6, 0, 0, 0, 1, 1 };
	int16_t decodeBuffer[SAMPLE_PER_FRAME];
	uint8_t *buf = inptr;
	*nbOut = 0;

again:	
	while (nbIn)
	{
		int dec_mode,packet_size;
		dec_mode = (buf[0] >> 3) & 0x000F;

		packet_size = block_size[dec_mode];

		if (packet_size > nbIn)
		{
			printf("Packet size %u, available data %u\n", packet_size, nbIn);
			return 1;
		}

		D_IF_decode(state, buf, decodeBuffer, _good_frame);

		// int to float
		int16_t *in = decodeBuffer;
		for (int i = 0; i < SAMPLE_PER_FRAME; i++) 
		{
			*(outptr++) = (float)*in / 32768;
			in++;
		}

		*nbOut += SAMPLE_PER_FRAME;

		buf += packet_size;
		nbIn -= packet_size;
	}

	return 1; 
}
