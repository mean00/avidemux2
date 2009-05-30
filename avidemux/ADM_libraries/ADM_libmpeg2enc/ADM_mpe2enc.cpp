/***************************************************************************
                          ADM_mpeg2enc.h  -  description
                             -------------------

		Interface to mpegenc

		pubits is patched to write to admwritebuffer
		readpic is patched to take info from buffer

    begin                : Tue Jun 4 2002
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
#include "ADM_default.h""

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "avi_vars.h"
#include "ADM_assert.h"

#include "prototype.h"


//#include "ADM_colorspace/colorspace.h"
#include "ADM_encoder/ADM_vidEncode.hxx"
//#include "ADM_codecs/ADM_divxEncode.h"
#include "ADM_mpeg2enc.h"
#include "ADM_encoder/adm_encffmatrix.h"

#include "ADM_default.h"
#include "prefs.h"


static uint8_t mpeg2_running=0;
#define MAX_BR_M 50*1000*1000
//#define TEST_NOB 1 // disable B frames


extern uint32_t mpeg2GetDts( void );

uint8_t Mpeg2enc::commonMpeg2Init( uint32_t qz, uint32_t maxbr ,uint32_t fps1000
				,uint8_t interlaced, uint8_t bff, uint8_t wide,	uint8_t fast)
{
    	if(!wide)
			_settings.aspect_ratio=MPG_4_3;
		else
			_settings.aspect_ratio=MPG_16_9;
		if(interlaced) {
			_settings.fieldenc=1;
            _settings.input_interlacing= (bff ? 2 : 1); // WLA
        }
		else
			_settings.fieldenc=0;
		if(maxbr==0)
			{
				_settings.bitrate=MAX_BR_M; // 20 megabits !	
				_settings.ignore_constraints=1;
			
			}			
		else
		{
			_settings.bitrate=maxbr;
			if(_settings.bitrate<11000) _settings.bitrate*=1000;	// bitrate
		}
		
		//printf("Dvd bt: %lu\n",maxbr);
		_settings.quant=qz;
		if(fast)
		{
			_settings._44_red=_settings._22_red=4;	// Turbo pass 1
			_settings.format=3;	
			_settings.video_buffer_size=4000;
		}
		mpeg2_running=1;
		if(fps1000==25000)	//PAL
		{
			_settings.min_GOP_size=_settings.max_GOP_size=15;
		}
		else
		{
			_settings.min_GOP_size=_settings.max_GOP_size=18;
		}
		return (uint8_t ) mpegenc_init(&_settings,(int) _w, (int)_h, (int) fps1000);
}

Mpeg2enc::Mpeg2enc( uint32_t width,uint32_t height) :encoder(width,height)
             				{
							memset(&_settings,0,sizeof(_settings));
							_settings.setDefault();
							_settings.fieldenc=0; // progressive
							_settings.searchrad=16; // speed up

                   			} ;
uint8_t Mpeg2enc::setQz( uint32_t quant)
{
	mpegenc_setQuantizer(quant);
	return 1;
}                   			
uint32_t Mpeg2enc::getCodedPictureNumber( void )
{
	return mpeg2GetDts();
}
uint8_t Mpeg2enc::disablePadding( void )
{
	_settings.noPadding=1;
}								
uint8_t Mpeg2enc::setMatrix(int matrix)
{

	switch(matrix)
	{
						case ADM_MATRIX_DEFAULT:
								_settings.hf_quant=0;
								printf("Default matrix\n");
								break;
						case ADM_MATRIX_TMP:
								_settings.hf_quant=4;
								break;
						case ADM_MATRIX_ANIME:
								printf("\n anime ** NOT AVAILABLE**\n");

								break;
						case ADM_MATRIX_KVCD:
								printf("\n using custom matrix : kvcd\n");
								_settings.hf_quant=3;
								break;
	}
	return 1;

}

										
					
Mpeg2enc::~Mpeg2enc()
{
	stopEncoder();
}
/*---------------------------------------------------------
		Mpeg2CBRcodec
---------------------------------------------------------
*/
uint8_t Mpeg2enc::init(  uint32_t qz, uint32_t maxbr, uint32_t fps1000,
			uint8_t interlaced, uint8_t bff, uint8_t wide,uint8_t fast) // WLA
{
		if(!wide)
			_settings.aspect_ratio=MPG_4_3;
		else
			_settings.aspect_ratio=MPG_16_9;
				
		mpeg2_running=1;
		return (uint8_t ) mpegenc_init(&_settings,(int) _w, (int)_h, (int) fps1000);


}
/*---------------------------------------------------------
		Mpeg2CBRcodec
---------------------------------------------------------
*/

 /*---------------------------------------------------------
		Mpeg2CBRcodec
---------------------------------------------------------
*/

uint8_t Mpeg2enc::encode(ADMImage *in,ADMBitstream *out)
{
int q,f,l;
uint8_t r;
	out->flags=0;
	f=(int)out->flags;
	l=(int)out->len;
	if(out->in_quantizer)
	    	setQz(out->in_quantizer);
	r=mpegenc_encode((char *)in->data,(char *)out->data,&l,&f,&q);
        out->out_quantizer=(uint32_t)q;
	out->flags=(uint32_t )f;
	out->len=(uint32_t)l;
	return r;

}
uint8_t Mpeg2enc::stopEncoder(void )
       {
       		if(mpeg2_running)
		{
			mpegenc_end();
		}
		mpeg2_running=0;

       }
/*--------------------------------------------------------------------------------*/

Mpeg2encVCD::Mpeg2encVCD( uint32_t width,uint32_t height) : Mpeg2enc(width,height)
{
uint32_t autosplit=0;
		if(!prefs->get(SETTINGS_MPEGSPLIT, &autosplit))
		{
			autosplit=690;		
		}

		// VCD
		_settings.format=1;
		_settings.fieldenc=0; // VCD are always progressive
		_settings.seq_length_limit=autosplit;
}

Mpeg2encSVCD::Mpeg2encSVCD( uint32_t width,uint32_t height) : Mpeg2enc(width,height)
{
uint32_t autosplit=0;
		if(!prefs->get(SETTINGS_MPEGSPLIT, &autosplit))
		{
			autosplit=690;		
		}

		// SVCD
		_settings.format=4;
		_settings.seq_length_limit=autosplit;
}
uint8_t Mpeg2encVCD::init(  uint32_t qz, uint32_t maxbr, uint32_t fps1000,
			uint8_t interlaced, uint8_t bff, uint8_t wide,uint8_t fast) // WLA
{
		if(!wide)
			_settings.aspect_ratio=MPG_4_3;
		else
			_settings.aspect_ratio=MPG_16_9;
				
		mpeg2_running=1;
		return (uint8_t ) mpegenc_init(&_settings,(int) _w, (int)_h, (int) fps1000);


}
uint8_t Mpeg2encSVCD::init( uint32_t qz, uint32_t maxbr ,uint32_t fps1000,
			uint8_t interlaced, uint8_t bff, uint8_t wide,
			uint8_t fast) // WLA
{
		return commonMpeg2Init(qz,maxbr,fps1000,interlaced,bff,wide,fast);
}

/*--------------------------------------------------------------------------------*/
Mpeg2encDVD::Mpeg2encDVD( uint32_t width,uint32_t height) : Mpeg2enc(width,height)
{
		// DVD
		_settings.format=9;
}
uint8_t Mpeg2encDVD::init( uint32_t qz, uint32_t maxbr ,uint32_t fps1000
				,uint8_t interlaced, uint8_t bff, uint8_t wide,	uint8_t fast) // WLA
{
		return commonMpeg2Init(qz,maxbr,fps1000,interlaced,bff,wide,fast);
}


