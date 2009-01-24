/***************************************************************************
                          ADM_deviceEsd.cpp  -  description

  ESD support as output audio device

    copyright            : (C) 2005 by mean
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
#include "ADM_audiodevice.h"


#include  "ADM_audiodevice.h"
#include  "ADM_audioDeviceInternal.h"

#include  "ADM_deviceEsd.h"
#include <esd.h>
#include <sys/time.h>
ADM_DECLARE_AUDIODEVICE(Esd,esdAudioDevice,1,0,1,"Esd audio device (c) mean");
/**
        \fn getLatencyMs
        \brief Returns device latency in ms
*/
uint32_t esdAudioDevice::getLatencyMs(void)
{
    return latency;
}
/**
    \fn localStop
    \brief

*/
bool  esdAudioDevice::localStop(void)
{
    if (esdDevice >= 0) {
        esd_close(esdDevice);
        esdDevice = -1;
    }
    if(esdServer>=0)
    {
        esd_close(esdServer);
        esdServer=-1;
    }
    return true;
}

/**
    \fn localInit
*/
bool esdAudioDevice::localInit(void)
{
esd_format_t format;

latency=0;
esd_server_info_t *esdInfo;

    esdServer=	esd_open_sound(NULL);
    if(esdServer>=0)
    {
        esdInfo = esd_get_server_info(esdServer);
        esd_print_server_info(esdInfo);

    }



    format=ESD_STREAM | ESD_PLAY | ESD_BITS16;
    if(_channels==1) format|=ESD_MONO;
        else format|=ESD_STEREO;

    printf("[ESD]  : %"LU" Hz, %"LU" channels\n", _frequency, _channels);
    esdDevice=esd_play_stream(format,_frequency,NULL,"avidemux");
    if(esdDevice<=0)
    {
        printf("[ESD] open failed\n");
        return 0;
    }
    printf("[ESD] open succeedeed\n");

    float l=esd_get_latency(esdServer);
    printf("[ESD] Raw latency %f\n",l);
    l=l/(44.1*4);
    latency=(uint32_t)l;

    printf("[ESD] Latency %u ms\n",latency);
    //printf("[ESD] get_esd_latency %u \n",esd_get_latency(esdDevice));
    return 1;
}

/**
    \fn sendData

*/
void esdAudioDevice::sendData(void)
{
	mutex.lock();
    uint32_t avail=wrIndex-rdIndex;
    if(avail>sizeOf10ms) avail=sizeOf10ms;
    mutex.unlock();
	int w=write(esdDevice, audioBuffer+rdIndex, avail);
    mutex.lock();
    rdIndex+=avail;
    mutex.unlock();
	return ;
}
//EOF
