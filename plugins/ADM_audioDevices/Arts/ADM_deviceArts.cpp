/***************************************************************************
                          ADM_deviceArts.cpp  -  description
                             -------------------
    begin                : Sat Sep 28 2002
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


#include <artsc.h>

#include  "ADM_audiodevice.h"
#include  "ADM_audioDeviceInternal.h"
#include  "ADM_deviceArts.h"

uint8_t artsInitialized=0;
ADM_DECLARE_AUDIODEVICE(Arts,artsAudioDevice,1,0,0,"Arts audio device (c) mean");
//_______________________________________________
//
//
//_______________________________________________
uint8_t  artsAudioDevice::stop(void) {
		if(!_stream)
		{
			printf("\n Arts: no stream\n");
			return 0;
		}

		arts_close_stream(_stream);
		// apparently arts 3.2 alpha does not like this
	   	//arts_free();
		_stream=NULL;
		printf("\n Arts stopped\n");
    	return 1;
}

//_______________________________________________
//
//
//_______________________________________________
uint8_t artsAudioDevice::init(uint32_t channels, uint32_t fq)
{
	_channels = channels;

    if(_stream)
    	{
			printf("\n purging previous instance\n");
			stop();
		}
    printf("\n Arts  : %lu Hz, %lu channels", fq, channels);
	if(!artsInitialized)
	{
		if(arts_init())
		{
			printf("\n Error initializing artsd\n");
			return 0;
		}
		artsInitialized=1;
	}

	_stream=arts_play_stream(fq, 16,channels, "Avidemux");

	if(!_stream)
	 {
		printf("\n Problem setting fq/channel, aborting\n");
		arts_free();
		return 0;
	}
	arts_stream_set(_stream, ARTS_P_BLOCKING, 1);
	arts_stream_set(_stream, ARTS_P_BUFFER_TIME, 50); // Ask for 1 sec buffer
	//arts_stream_set(_stream,  ARTS_P_PACKET_SETTINGS, (11<<16)+10);


    return 1;
}

//_______________________________________________
//
//
//_______________________________________________
uint8_t artsAudioDevice::play(uint32_t len, float *data)
 {

	if(!_stream) return 0;

	dither16(data, len, _channels);

	return arts_write(_stream, data, len*2);
}

/*
** JSC     Mon Nov 28 19:20:06 CET 2005
** based on http://www.arts-project.org/doc/mcop-doc/artsd-faq.html
** arts only works with OSS devices (or alsa devices using oss emulation)
** so we only need OSS support here
*/
uint8_t artsAudioDevice::setVolume(int volume){
#ifdef OSS_SUPPORT
        ossAudioDevice dev;
        dev.setVolume(volume);
#endif
        return 1;
}

#else
void dummy_art_func( void);
void dummy_art_func( void)
 {
}
#endif
