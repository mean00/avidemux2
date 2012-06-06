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
#include <opencore-amrnb/interf_dec.h>
};

#define ADM_AMR_BUFFER (16 * 1024) // 16 kB internal

class ADM_AudiocodecOpencoreAmrNb : public ADM_Audiocodec
{
protected:
	void *state;
	uint8_t _buffer[ADM_AMR_BUFFER];
	uint32_t _head, _tail;

public:
	ADM_AudiocodecOpencoreAmrNb(uint32_t fourcc, WAVHeader *info,uint32_t extraLength,uint8_t *extraDatab);
	~ADM_AudiocodecOpencoreAmrNb();
	bool    resetAfterSeek(void);
	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
	uint8_t isCompressed(void) { return 1; }
	uint8_t isDecompressable(void) { return 1; }
};

static  ad_supportedFormat Formats[]={
        {WAV_AMRNB,AD_MEDIUM_QUAL},
};

DECLARE_AUDIO_DECODER(ADM_AudiocodecOpencoreAmrNb,	// Class
1,0,0, 												// Major, minor,patch 
Formats, 											// Supported formats
"opencore-amrnb decoder plugin for Avidemux (c) Mean/Gruntster\n"); 	// Desc

bool ADM_AudiocodecOpencoreAmrNb::resetAfterSeek( void ) 
{
	_tail = _head = 0;

	return 1;
};

uint8_t ADM_AudiocodecOpencoreAmrNb::endDecompress( void ) 
{
	_tail = _head = 0;
	return 1;
};

ADM_AudiocodecOpencoreAmrNb::ADM_AudiocodecOpencoreAmrNb(
	uint32_t fourcc, WAVHeader *info, uint32_t extraLength, uint8_t *extraData) : ADM_Audiocodec(fourcc,*info)
{
	state = Decoder_Interface_init();
}

ADM_AudiocodecOpencoreAmrNb::~ADM_AudiocodecOpencoreAmrNb( )
{
	if (state)
	{
		Decoder_Interface_exit(state);
		state = NULL;
	}
}

uint8_t ADM_AudiocodecOpencoreAmrNb::run(uint8_t *inptr, uint32_t nbIn, float *outptr,   uint32_t *nbOut)
{
#define SAMPLE_PER_FRAME 160

	static const short block_size[16]={ 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };
	int16_t decodeBuffer[SAMPLE_PER_FRAME];
	uint8_t *buf = inptr;
	*nbOut = 0;

again:	
	while (nbIn)
	{
		int dec_mode,packet_size;
		dec_mode = (buf[0] >> 3) & 0x000F;

		packet_size = block_size[dec_mode] + 1;
		if (packet_size > nbIn)
		{
			printf("Packet size %u, available data %u\n", packet_size, nbIn);
			return 1;
		}

		Decoder_Interface_Decode(state, buf, decodeBuffer, 0);

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
