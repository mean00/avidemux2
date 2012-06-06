/**
    \file ADM_ad_ulaw.cpp
    \brief Audio decoders built ulaw from mplayer or ffmpeg (??, can't remember)
    \author mean (c) 2009

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include <math.h>

#include "ADM_default.h"
#include "ADM_ad_plugin.h"

/**
    \class ADM_AudiocodecUlaw
    \brief
*/

 class ADM_AudiocodecUlaw : public     ADM_Audiocodec
 {
 	public:
		ADM_AudiocodecUlaw(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d);
		virtual	~ADM_AudiocodecUlaw() ;
		virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
		virtual	uint8_t isCompressed(void) {return 1;}
};
// Supported formats + declare our plugin
//*******************************************************
static  ad_supportedFormat Formats[]={
        {WAV_ULAW,AD_MEDIUM_QUAL},
};

DECLARE_AUDIO_DECODER(ADM_AudiocodecUlaw,						// Class
			0,0,1, 												// Major, minor,patch
			Formats, 											// Supported formats
			"Ulaw decoder plugin for avidemux (c) Mean\n"); 	// Desc
//********************************************************

static int expon [8]= {0,132,396,924,1980,4092,8316,16764};
/**
    \fn ADM_AudiocodecUlaw
    \brief
*/
ADM_AudiocodecUlaw::ADM_AudiocodecUlaw(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d) 
: ADM_Audiocodec(fourcc,*info)
{
		
}
/**
    \fn ~ADM_AudiocodecUlaw
    \brief
*/

ADM_AudiocodecUlaw::~ADM_AudiocodecUlaw()
{

}
/**
    \fn run
    \brief
*/


uint8_t ADM_AudiocodecUlaw::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
uint8_t byt1;
int16_t out;

int mant,mexp,sign;

	*nbOut=nbIn;

	for(uint32_t i=0;i<nbIn;i++)
	{
		byt1=*inptr++;
		byt1=~byt1;
		sign=(byt1&0x80);
		mexp=(byt1>>4)&0x7;
		mant=byt1&0xf;
		out=expon[mexp]+(mant<<(mexp+3));
		if(sign) out=-out;
		*outptr++=(float)out / 32768;
	}

	return 1;
}
// EOF
