/***************************************************************************
                          ADM_deviceoss.cpp  -  description
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
#include <unistd.h>
#include "ADM_default.h"


#if defined(ADM_BSD_FAMILY) && !defined(__FreeBSD__) && !defined(__OpenBSD__)
	#include <soundcard.h>
	const char *dsp = DEVOSSAUDIO;;

#elif defined(__OpenBSD__)
	#include <soundcard.h>
	const char *dsp = "/dev/audio";
	const char *device_mixer = "/dev/mixer";

#else
	#include <sys/soundcard.h>
	const char *dsp = "/dev/dsp";
        const char *device_mixer = "/dev/mixer";
#endif
#include  "ADM_audiodevice.h"
#include  "ADM_audioDeviceInternal.h"

#include <errno.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include  "ADM_deviceoss.h"

ADM_DECLARE_AUDIODEVICE(Oss,ossAudioDevice,1,0,0,"Oss audio device (c) mean");

//_______________________________________________
//
//
//_______________________________________________
uint8_t  ossAudioDevice::setVolume(int volume) 
{
        int fd;
	int ret;
	uint32_t which_vol = 0;

	//prefs->get(FEATURE_AUDIOBAR_USES_MASTER,&which_vol);
        fd=open(device_mixer,O_RDONLY);
        if(!fd)
        {
                printf("[OSSS]: cannot open mixer\n");
                return 0;
        }
        printf("[OSSS]: New %s volume %d\n",(which_vol?"master":"pcm"),volume);
        // Assuming stereo
        volume=volume+(volume<<8);
	if( which_vol ){
        	ret = ioctl(fd, MIXER_WRITE(SOUND_MIXER_VOLUME), &volume);
	}else{
        	ret = ioctl(fd, MIXER_WRITE(SOUND_MIXER_PCM   ), &volume);
	}
        close(fd);

	if( ret ){
		if( errno == EBADF ){
			printf("[OSSS]set mixer failed: %u (possible access issue)\n",errno);
		}else{
			printf("[OSSS]set mixer failed: %u\n",errno);
		}
	}
        return 1;

}

/**
    \fn localStop

*/
bool  ossAudioDevice::localStop(void) 
{
    if (oss_fd > 0) {
       // ioctloss_fd, SNDCTL_DSP_SKIP, NULL); // Flush
        close(oss_fd);
        oss_fd = 0;
    }
    return true;
}


/**
    \fn localInit
*/
bool ossAudioDevice::localInit(void) 
{
	
    int fq=_frequency;
    printf("[OSSS]: %"PRIu32" Hz, %"PRIu32" channels\n", _frequency, _channels);
    // open OSS device
    oss_fd = open(dsp, O_WRONLY /*| O_NONBLOCK*/);
    if (oss_fd == -1) {
/*
	if( errno == EACCES )
	{
          GUI_Error_HIG(QT_TR_NOOP("Could not open OSS audio device"), QT_TR_NOOP("Check the permissions for /dev/dsp."));
	  }
	else
*/
           printf("[OSSS] Error initializing OSS: Error : %d\n", errno);
        return false;
    }
    // seems ok, set up audio 
    if (ioctl (oss_fd, SNDCTL_DSP_SPEED, &fq) < 0) {
        printf("[OSSS] Error setting up OSS(SPEED): Error : %d\n", errno);
        return 0;
    }
    if (_channels > 2) {
        if (ioctl (oss_fd, SNDCTL_DSP_CHANNELS, &_channels) < 0) {
	    printf("[OSSS] Error setting up OSS(CHANNELS): Error : %d\n", errno);
	    return 0;
        }
    } else {
        int chan = _channels - 1;
        if (ioctl (oss_fd, SNDCTL_DSP_STEREO, &chan) < 0) {
	    printf("[OSSS] Error setting up OSS(STEREO): Error : %d\n", errno);
	    return 0;
        }
    }
#ifdef ADM_BIG_ENDIAN    
    int fmt = AFMT_S16_BE;
#else
    int fmt = AFMT_S16_LE;
#endif    
    if (ioctl (oss_fd, SNDCTL_DSP_SETFMT, &fmt) < 0) {
        printf("[OSSS] Error setting up OSS(FORMAT): Error : %d\n", errno);
        return false;
    }

    return true;
}

/**
     \fn sendData
     \brief Send data to playback
*/
void    ossAudioDevice::sendData(void)
{
    int pack=_channels*_frequency*2;
    int w;
    pack/=100;
    pack&=~3;   // Try to send 10 ms chunks

	mutex.lock();
    if(wrIndex-rdIndex<pack) pack=wrIndex-rdIndex;
    mutex.unlock();
    w = write(oss_fd, audioBuffer+rdIndex, pack);
    mutex.lock();
    rdIndex+=pack;
    mutex.unlock();
    if(w!=pack) printf("[OSS] Error :%u vs %u\n",w,pack);
    ADM_usleep(1000);
    return;
   
}
/**
    \fn getWantedChannelMapping
*/
const CHANNEL_TYPE mono[MAX_CHANNELS]={ADM_CH_MONO};
const CHANNEL_TYPE stereo[MAX_CHANNELS]={ADM_CH_FRONT_LEFT,ADM_CH_FRONT_RIGHT};
const CHANNEL_TYPE fiveDotOne[MAX_CHANNELS]={ADM_CH_FRONT_LEFT,ADM_CH_FRONT_RIGHT,ADM_CH_FRONT_CENTER,
                                             ADM_CH_REAR_LEFT,ADM_CH_REAR_RIGHT,ADM_CH_LFE};
const CHANNEL_TYPE *ossAudioDevice::getWantedChannelMapping(uint32_t channels)
{
    switch(channels)
    {
        case 1: return mono;break;
        case 2: return stereo;break;
        default:
                return fiveDotOne;
                break;
    }
    return NULL;
}
//EOF
