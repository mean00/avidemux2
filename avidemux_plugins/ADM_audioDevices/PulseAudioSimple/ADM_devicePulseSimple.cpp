/***************************************************************************
                          ADM_devicePulseSimple.cpp  -  description

  Simple Pulse audio out
                          
    copyright            : (C) 2008 by mean
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

#include  "ADM_audioDeviceInternal.h"
#include  "ADM_devicePulseSimple.h"
#include  "pulse/simple.h"
#include  "pulse/error.h"

ADM_DECLARE_AUDIODEVICE(PulseAudioS,pulseSimpleAudioDevice,1,0,2,"PulseAudioSimple audio device (c) mean");
#define INSTANCE  ((pa_simple *)instance)

// By default we use float NOT
#define ADM_PULSE_INT16
#define ADM_PULSE_LATENCY 50 // ms
/**
    \fn pulseSimpleAudioDevice
    \brief Constructor

*/
pulseSimpleAudioDevice::pulseSimpleAudioDevice()
{
    instance=NULL;
    latency=0;
}
/**
    \fn pulseSimpleAudioDevice
    \brief Returns delay in ms
*/
uint32_t pulseSimpleAudioDevice::getLatencyMs(void)
{
   return ADM_PULSE_LATENCY; //latency;
}

/**
    \fn localStop
    \brief stop & release device

*/

bool  pulseSimpleAudioDevice::localStop(void) 
{
int er;
    if(!instance) return 1;
    ADM_assert(instance);
    pa_simple_flush(INSTANCE,&er);
    pa_simple_free(INSTANCE);
    instance=NULL;
    printf("[PulseAudio] Stopped\n");
    return 1;
}

/**
    \fn    localInit
    \brief Take & initialize the device

*/
bool pulseSimpleAudioDevice::localInit(void) 
{
ADM_info("Pulse, initiliazing channel=%d samplerate=%d\n",(int)_channels,(int)_frequency);
pa_simple *s;
pa_sample_spec ss;
int er;
pa_buffer_attr attr;
pa_channel_map map,*pmap=NULL;
    attr.maxlength = (uint32_t) -1;
    attr.tlength = (uint32_t )-1;
    attr.prebuf =(uint32_t) -1;
    attr.minreq = (uint32_t) -1;
    attr.fragsize =(uint32_t) -1;

  // We want something like 20 ms latency
   uint64_t bufSize=_frequency;
            bufSize*=_channels;
            bufSize*=2;      // 1 second worth of audio

  bufSize=bufSize/1000;
  bufSize*=ADM_PULSE_LATENCY;
  attr.tlength=bufSize;       // Latency in bytes
 

  // Channel mapping
  if(_channels>2)
    {
        pmap=&map;
        map.channels=_channels;
        map.map[0]=PA_CHANNEL_POSITION_FRONT_LEFT;
        map.map[1]=PA_CHANNEL_POSITION_FRONT_RIGHT;
        map.map[2]=PA_CHANNEL_POSITION_FRONT_CENTER;
        map.map[3]=PA_CHANNEL_POSITION_REAR_LEFT;
        map.map[4]=PA_CHANNEL_POSITION_REAR_RIGHT;
        map.map[5]=PA_CHANNEL_POSITION_SUBWOOFER;
  }

  ss.format = PA_SAMPLE_S16LE;
  ss.channels = _channels;
  ss.rate =_frequency;
 
  instance= pa_simple_new(NULL,               // Use the default server.
                    "Avidemux2",           // Our application's name.
                    PA_STREAM_PLAYBACK,
                    NULL,               // Use the default device.
                    "Sound",            // Description of our stream.
                    &ss,                // Our sample format.
                    pmap,               // Use default channel map
                    &attr ,             // Use default buffering attributes.
                    &er               // Ignore error code.
                    );
  if(!instance)
    {
        ADM_info("[PulseSimple] open failed :%s\n",pa_strerror(er));
        return 0;
    }
#if 0
    pa_usec_t l=0;
    // Latency...
    Clock    ticktock;
    ticktock.reset();
    if(0>pa_simple_write(INSTANCE,silence, sizeOf10ms,&er))
    {
      fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(er));
    }
    pa_simple_drain(INSTANCE,&er);
    latency=ticktock.getElapsedMS();
    ADM_info("[Pulse] Latency :%"LU", total %"LU"\n",latency,pa_simple_get_latency(INSTANCE,&er)/1000);
#endif
    ADM_info("[PulseSimple] open ok\n");
    return 1;

}

/**
    \fn sendData
    \brief Playback samples

*/
void pulseSimpleAudioDevice::sendData(void)
{
int er;
    if(!instance) return ;
	
    mutex.lock();
    uint32_t avail=wrIndex-rdIndex;
    if(!avail)
    {
        mutex.unlock();
        pa_simple_write(INSTANCE,silence, sizeOf10ms,&er);
        
        return ;
    }
    if(avail>sizeOf10ms) avail=sizeOf10ms;
    
    uint8_t *data=audioBuffer+rdIndex;
    mutex.unlock();
    pa_simple_write(INSTANCE,data, avail,&er);
    mutex.lock();
    rdIndex+=avail;
    mutex.unlock();
	return ;

}
/**
    \fn getWantedChannelMapping
*/
const CHANNEL_TYPE mono[MAX_CHANNELS]={ADM_CH_MONO};
const CHANNEL_TYPE stereo[MAX_CHANNELS]={ADM_CH_FRONT_LEFT,ADM_CH_FRONT_RIGHT};
const CHANNEL_TYPE fiveDotOne[MAX_CHANNELS]={ADM_CH_FRONT_LEFT,ADM_CH_FRONT_RIGHT,ADM_CH_FRONT_CENTER,
                                             ADM_CH_REAR_LEFT,ADM_CH_REAR_RIGHT,ADM_CH_LFE};
const CHANNEL_TYPE *pulseSimpleAudioDevice::getWantedChannelMapping(uint32_t channels)
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
