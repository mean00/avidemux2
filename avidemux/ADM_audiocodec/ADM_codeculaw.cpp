//
// C++ Implementation: ADM_codeculaw
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//


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

//#include "avifmt.h"
//#include "avifmt2.h"
//#include "fourcc.h"

#include "ADM_audiocodec/ADM_audiocodec.h"


static int expon [8]= {0,132,396,924,1980,4092,8316,16764};
ADM_AudiocodecUlaw::ADM_AudiocodecUlaw( uint32_t fourcc,WAVHeader *info) : ADM_Audiocodec(fourcc)
{
		
}
ADM_AudiocodecUlaw::~ADM_AudiocodecUlaw()
{

}


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

