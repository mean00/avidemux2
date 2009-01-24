/***************************************************************************
                          ADM_deviceAlsa.cpp  -  description
                             -------------------

	Strongly derivated from code sample from alsa-project.org with some bits
		from mplayer concerning the swparams

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
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>




#include <alsa/asoundlib.h>
#include  "ADM_audiodevice.h"
#include  "ADM_audioDeviceInternal.h"


#ifdef ADM_ADEVICE_DMIX
#define alsaAudioDevice alsaAudioDeviceDmix
#include  "ADM_deviceALSA.h"
ADM_DECLARE_AUDIODEVICE(AlsaDmix,alsaAudioDevice,1,0,0,"Alsa Audio Device (dmix) (c) Mean 2008");
#define ADEVICE "dmix"
#endif

#if ADM_ADEVICE_HW
#define alsaAudioDevice alsaAudioDeviceHw0
#include  "ADM_deviceALSA.h"

ADM_DECLARE_AUDIODEVICE(AlsaHw0,alsaAudioDevice,1,0,0,"Alsa Audio Device (hw:0) (c) Mean 2008");
#define ADEVICE "hw:0"
#endif



/* Handle for the PCM device */
snd_pcm_t *pcm_handle;

    alsaAudioDevice::alsaAudioDevice( void )
    {
		_init=0;
    }
/**
    \fn localInit
    \brief
*/
bool alsaAudioDevice::localInit( void )
{
	int dir=0;


	_init=0;
   /* Playback stream */
    snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;

    /* This structure contains information about    */
    /* the hardware and can be used to specify the  */
    /* configuration to be used for the PCM stream. */
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams;


  static char *pcm_name;
//  if( prefs->get(DEVICE_AUDIO_ALSA_DEVICE, &pcm_name) != RC_OK )
               pcm_name = ADM_strdup(ADEVICE);
    printf("[Alsa] Using device :%s\n",pcm_name);
 /* Allocate the snd_pcm_hw_params_t structure on the stack. */
    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);
    /* Open PCM. The last parameter of this function is the mode. */
    /* If this is set to 0, the standard mode is used. Possible   */
    /* other values are SND_PCM_NONBLOCK and SND_PCM_ASYNC.       */
    /* If SND_PCM_NONBLOCK is used, read / write access to the    */
    /* PCM device will return immediately. If SND_PCM_ASYNC is    */
    /* specified, SIGIO will be emitted whenever a period has     */
    /* been completely processed by the soundcard.                */
    if (snd_pcm_open(&pcm_handle, pcm_name, stream, 0*SND_PCM_NONBLOCK) < 0) {
      fprintf(stderr, "[Alsa]Error opening PCM device %s\n", pcm_name);
      return(0);
    }
    // past this point we got _init=1 -> partially initialized
    _init=1;
      /* Init hwparams with full configuration space */
    if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0) {
      fprintf(stderr, "[Alsa]Can not configure this PCM device.\n");
      ADM_dealloc(pcm_name);
      return(0);
    }
    ADM_dealloc(pcm_name);
    /* Set access type. This can be either    */
    /* SND_PCM_ACCESS_RW_INTERLEAVED or       */
    /* SND_PCM_ACCESS_RW_NONINTERLEAVED.      */
    /* There are also access types for MMAPed */
    /* access, but this is beyond the scope   */
    /* of this introduction.                  */
    if (snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
      fprintf(stderr, "[Alsa]Error setting access.\n");
      return(0);
    }

    /* Set sample format */
//    if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_FLOAT) < 0) { //need more test
    //  fprintf(stderr, "Error setting float format.\n");

#ifdef ADM_BIG_ENDIAN
    if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S16_BE) < 0)
#else
    if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S16_LE) < 0)
#endif
    {
      fprintf(stderr, "[Alsa]Error setting format.\n");
      return(0);
    }
	//}
    /* Set sample rate. If the exact rate is not supported */
    /* by the hardware, use nearest possible rate.         */
    int exact_rate;
    dir=0;
    exact_rate = snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &_frequency, &dir);
    if (dir != 0) {
      fprintf(stderr, "[Alsa]The rate %"LU" Hz is not supported by your hardware.\n  ==> Using %d Hz instead.\n", _frequency, exact_rate);
    }

    /* Set number of channels */
    if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, _channels) < 0) {
      fprintf(stderr, "[Alsa]Error setting channels.\n");
      return(0);
    }
#if 0
    	uint32_t periods= _frequency*2*channel*10;
	uint32_t periodsize=1;
    /* Set number of periods. Periods used to be called fragments. */
    if (snd_pcm_hw_params_set_periods(pcm_handle, hwparams, periods, 0) < 0) {
      fprintf(stderr, "[Alsa]Error setting periods.\n");
      return(0);
    }
#else

 	unsigned int buffer_time = 800000;
	int er;
	unsigned int buff;
	dir=0;

	if ((er=snd_pcm_hw_params_set_buffer_time_near(pcm_handle, hwparams, &buffer_time, &dir)) < 0)
	  {
	    printf("[Alsa]Error : hw_params_set_buffer_time\n");
	    return(0);
	  }
	  // unsigned ?
	  dir=0;
	  buff=buffer_time>>2;
	snd_pcm_hw_params_set_period_time_near(pcm_handle, hwparams, &buff, &dir) ;
#if 0
	if (snd_pcm_hw_params_set_period_time_near(pcm_handle, hwparams, buffer_time>>2, 0) < 0)
	  /* original: alsa_buffer_time/ao_data.bps */
	  {
	    printf("[Alsa]Error : hw_params_set_period_time\n");
	    return(0);
	  }
#endif
#endif


/*
If your hardware does not support a buffersize of 2^n, you can use the function snd_pcm_hw_params_set_buffer_size_near. This works similar to snd_pcm_hw_params_set_rate_near. Now we apply the configuration to the PCM device pointed to by pcm_handle. This will also prepare the PCM device.
*/


    /* Apply HW parameter settings to */
    /* PCM device and prepare device  */
    if (snd_pcm_hw_params(pcm_handle, hwparams) < 0) {
      fprintf(stderr, "[Alsa]Error setting HW params.\n");
      return(0);
    }

 	if (snd_pcm_sw_params_current(pcm_handle, swparams) < 0)
	    {
	      printf("[Alsa]Error setting SW params.\n");
	      return(0);
	    }

 	// be sure that playback starts immediatly (or near)
	  if (snd_pcm_sw_params_set_avail_min(pcm_handle, swparams, 4) < 0)
	    {
	      printf("[Alsa]Error setting set_avail_min \n");
	      return(0);
	    }

	  if (snd_pcm_sw_params(pcm_handle, swparams) < 0)
	    {
	      printf("[Alsa]Error:snd_pcm_sw_params\n ");
	      return(0);
	    }


      if ( snd_pcm_prepare(pcm_handle) < 0)
	{
	  printf("[Alsa]Error : snd_pcm_prepare\n");
	  return(0);
	}

        printf("[Alsa]Success initializing: fq :%u channel %u\n", _frequency,_channels);

    // 2=fully initialized
    _init=2;
    return 1;
}

/**
    \fn sendData

*/
void alsaAudioDevice::sendData(void)
{
	/* Write num_frames frames from buffer data to    */
	/* the PCM device pointed to by pcm_handle.       */
	/* Returns the number of frames actually written. */
    if(2!=_init) return ;
    uint32_t lenInBytes,lenInSample;
    lenInBytes=sizeOf10ms*2; // 20 ms at a time
    mutex.lock();
    uint32_t avail;
_again:

	avail=wrIndex-rdIndex;

    if(lenInBytes>avail) lenInBytes=avail;
    lenInSample=lenInBytes/(_channels*2);
    if(!lenInSample)
    {
        printf("[Alsa] Underflow\n");
        mutex.unlock();
        return ;
    }
        uint8_t *start=audioBuffer+rdIndex;
        int ret;

        mutex.unlock(); // There is a race here....
       	ret=snd_pcm_writei(pcm_handle,start, lenInSample);
        mutex.lock();
		if(ret==(int)lenInSample)
		{
			rdIndex+=lenInSample*2*_channels;
            mutex.unlock();
            return ;
		}

		if(ret<0)
		{
			switch(ret)
			{
				case    -EAGAIN :
					//wait a bit to flush datas
					printf("[Alsa]ALSA EAGAIN\n");
					snd_pcm_wait(pcm_handle, 1000);
					goto _again;

				case    -EPIPE:
					printf("[Alsa]ALSA EPIPE\n");
					snd_pcm_prepare(pcm_handle);
					goto _again;
				default:
					printf("[Alsa]ALSA Error %d : Play %s (len=%d)\n",ret, snd_strerror(ret),0);

			}
		}

	mutex.unlock();
	return ;
}
/**
    \fn localStop

*/
 bool alsaAudioDevice::localStop( void )
 {
 // we have at least a partial initialization
 if(_init)
 {
       /* Stop PCM device and drop pending frames */
    snd_pcm_drop(pcm_handle);

    /* Stop PCM device after pending frames have been played */
    snd_pcm_drain(pcm_handle);
      if (snd_pcm_close(pcm_handle) < 0)
      {
		printf("[Alsa] Troubles closing alsa\n");

      }
     }
     _init=0;
     return true;
}

uint8_t alsaAudioDevice::setVolume(int volume){
  snd_mixer_t *mixer_handle;
  char *pcm_name;
  uint32_t which_vol;
  int rc;
/*
	if( prefs->get(DEVICE_AUDIO_ALSA_DEVICE, &pcm_name) != RC_OK )
		pcm_name = ADM_strdup("hw:0");
	if( prefs->get(FEATURE_AUDIOBAR_USES_MASTER,&which_vol) != RC_OK )
		which_vol = 0;
*/
    pcm_name = ADM_strdup("hw:0");
    which_vol = 0;
	if( (rc=snd_mixer_open(&mixer_handle,0)) < 0 ){
		printf("[Alsa]: snd_mixer_open failed: %d\n",rc);
		ADM_dealloc(pcm_name);
		return 0;
	}
// MEANX: Cannot use the real name, does not work with dmix
	if( (rc=snd_mixer_attach(mixer_handle,"hw:0")) < 0 ){
		printf("[Alsa]: snd_mixer_attach failed: %d, %s\n",rc, snd_strerror (rc));
		snd_mixer_close(mixer_handle);
		ADM_dealloc(pcm_name);
		return 0;
	}
	ADM_dealloc(pcm_name);
	if( (rc=snd_mixer_selem_register(mixer_handle,NULL,NULL)) < 0 ){
		printf("[Alsa]: snd_mixer_selem_register failed: %d\n",rc);
		snd_mixer_close(mixer_handle);
		return 0;
	}
	if( (rc=snd_mixer_load(mixer_handle)) < 0 ){
		printf("[Alsa]: snd_mixer_load failed: %d\n",rc);
		snd_mixer_close(mixer_handle);
		return 0;
	}
	{ snd_mixer_elem_t *elem;
	  snd_mixer_selem_id_t *sid;
	  const char *str;
		snd_mixer_selem_id_alloca(&sid);
		for (elem = snd_mixer_first_elem(mixer_handle);
		     elem;
		     elem = snd_mixer_elem_next(elem)) {
			snd_mixer_selem_get_id(elem, sid);
			str = snd_mixer_selem_id_get_name(sid);
			if( (which_vol == 0 && !strcmp(str,"PCM"))   ||
			    (which_vol == 1 && !strcmp(str,"Master"))  ){
			  long val=0, min=0, max=0;
				snd_mixer_selem_get_playback_volume_range(elem,&min,&max);
				/*
				if( (rc=snd_mixer_selem_get_playback_volume(elem,SND_MIXER_SCHN_FRONT_LEFT,&val)) < 0 ){
					printf("ALSA: snd_mixer_selem_get_playback_volume failed: %d\n",rc);
				}
				printf("ALSA: old val: %lu\n",val*100/max);
				*/
				if( (rc=snd_mixer_selem_set_playback_volume_all(elem,volume*max/100)) < 0 ){
					printf("[Alsa]: snd_mixer_selem_set_playback_volume_all failed: %d\n",rc);
				}
				printf("[Alsa]: new %s val: %"LU"\n",(which_vol?"master":"pcm"),volume);
				break;
			}
		}
	}
	snd_mixer_close(mixer_handle);
	return 0;
}

//EOF
