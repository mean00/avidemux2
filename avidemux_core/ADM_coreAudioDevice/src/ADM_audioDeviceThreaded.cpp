/***************************************************************************
    \file ADM_audioDeviceThreaded.cpp
    \brief Base class for audio playback with a dedicated thread

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
#include "ADM_audioDeviceInternal.h"
#include "math.h"

const char *i2state(int a)
{
    if(a==AUDIO_DEVICE_STARTED) return "DeviceStarted";
    if(a==AUDIO_DEVICE_STOPPED) return "DeviceStopped";
    if(a==AUDIO_DEVICE_STOP_REQ) return "DeviceStop_Requested";
    if(a==AUDIO_DEVICE_STOP_GR) return "DeviceStop_Granted";
    return "?????";

}
#define CHANGE_STATE(x) { printf("%s -> %s\n",i2state(stopRequest),i2state(x));stopRequest=x;}
/**
    \fn audioDeviceThreaded
    \brief constructor
*/
audioDeviceThreaded::audioDeviceThreaded(void)
{
    audioBuffer=NULL;
    silence=NULL;
}
/**
    \fn audioDeviceThreaded
    \brief destructor
*/
audioDeviceThreaded::~audioDeviceThreaded()
{
   // PURE! stop();
}
/**
    \fn bouncer
    \brief trampoline function
*/
static void *bouncer(void *in)
{
audioDeviceThreaded *device=(audioDeviceThreaded *)in;
    device->Loop();
    return NULL;
}
/**

*/
void audioDeviceThreaded::Loop(void)
{
    printf("[AudioDeviceThreaded] Entering loop\n");
    while(stopRequest==AUDIO_DEVICE_STARTED)
    {

        sendData();

    }
    CHANGE_STATE(AUDIO_DEVICE_STOP_GR);
    printf("[AudioDeviceThreaded] Exiting loop\n");
}
/**
    \fn audioDeviceThreaded
    \brief destructor
*/
uint8_t audioDeviceThreaded::init(uint32_t channel, uint32_t fq ,CHANNEL_TYPE *channelMapping)
{
    // Allocate buffer
    memcpy(incomingMapping,channelMapping,sizeof(CHANNEL_TYPE)*MAX_CHANNELS);
    _channels=channel;
    _frequency=fq;
    sizeOf10ms=(_channels*_frequency*2)/100;
    sizeOf10ms&=~15; // make sure it is a multiple of 16
    silence=new uint8_t[sizeOf10ms];
    memset(silence,0,sizeOf10ms);
    audioBuffer=new uint8_t[ADM_THREAD_BUFFER_SIZE];
    rdIndex=wrIndex=0;
    CHANGE_STATE(AUDIO_DEVICE_STOPPED);
    //
    if(!localInit()) return 0;
    // Spawn
    CHANGE_STATE(AUDIO_DEVICE_STARTED);
    ADM_assert(!pthread_create(&myThread,NULL,bouncer,this));


    return 1;
}
/**
    \fn getBufferFullness
    \brief Returns the number of 10 ms of audio in the buffer

*/
uint32_t   audioDeviceThreaded:: getBufferFullness(void)
{
    mutex.lock();
    float nbBytes=wrIndex-rdIndex;
    mutex.unlock();
    nbBytes/=sizeOf10ms;
    return 1+(uint32_t)nbBytes;
}
/**
    \fn stop
    \brief stop
*/
uint8_t audioDeviceThreaded::stop()
{
 uint32_t maxWait=3*1000; // Should be enough to drain
    ADM_info("[audioDevice]Stopping device...");
    if(stopRequest==AUDIO_DEVICE_STARTED)
    {
        CHANGE_STATE(AUDIO_DEVICE_STOP_REQ);
        while(stopRequest==AUDIO_DEVICE_STOP_REQ && maxWait)
        {
            ADM_usleep(1000);
            maxWait--;
        }
    }
    if(!maxWait)
    {
        ADM_error("Audio device did not stop cleanly\n");
    }
    localStop();
    if(audioBuffer)
    {
        delete [] audioBuffer;
        audioBuffer=NULL;
    }
    if(silence) delete [] silence;
    silence=NULL;
    CHANGE_STATE(AUDIO_DEVICE_STOPPED);
    return 1;
}
/**
    \fn write
    \brief We assume that we have full channels each time

*/
bool        audioDeviceThreaded::writeData(uint8_t *data,uint32_t lenInByte)
{
    mutex.lock();
    if(wrIndex>ADM_THREAD_BUFFER_SIZE/2 && rdIndex>ADM_THREAD_BUFFER_SIZE/4)
    {
        memmove(audioBuffer,audioBuffer+rdIndex,wrIndex-rdIndex);
        wrIndex-=rdIndex;
        rdIndex=0;
    }
    if(wrIndex+lenInByte>ADM_THREAD_BUFFER_SIZE)
    {
        printf("[AudioDevice] Overflow rd:%"LU"  start(wr):%u len%u limit%u\n",rdIndex,wrIndex,lenInByte,ADM_THREAD_BUFFER_SIZE);
        mutex.unlock();
        return false;
    }

    memcpy(audioBuffer+wrIndex,data,lenInByte);
   
    wrIndex+=lenInByte;
    mutex.unlock();
    return true;
}
/**
    \fn read
*/
bool        audioDeviceThreaded::readData(uint8_t *data,uint32_t lenInByte)
{
    mutex.lock();
    if(wrIndex-rdIndex<lenInByte)
    {
        printf("[AudioDevice] Underflow, wanted %u, only have %u\n",lenInByte,wrIndex-rdIndex);
        return false;
    }
    memcpy(data,audioBuffer+rdIndex,lenInByte);
    rdIndex+=lenInByte;
    mutex.unlock();
    return true;
}
/**
    \fn play

*/
uint8_t     audioDeviceThreaded::play(uint32_t len, float *data)
{
    // Reorder channels if needed...
    uint32_t samples=len;
    samples/=_channels;
    ADM_audioReorderChannels(_channels,
                    (float *)(data),
                    samples,
                    incomingMapping,
                    (CHANNEL_TYPE*)getWantedChannelMapping(_channels));
    // dither to int16_t
	dither16(data, len, _channels);
    len*=2;
    // Store in buffer
    return writeData((uint8_t *)data,len);

}
/**
    \fn getVolumeStats
    \brief Return stats about volume for each channel [6] between 0 & 255
*/
bool        audioDeviceThreaded::getVolumeStats(uint32_t *vol)
{
    float f[6];
    uint32_t raw[6];
    memset(vol,0,sizeof(uint32_t)*6);
    // 5 ms should be enough, i.e. fq/200
    uint32_t samples=_frequency/200;
    mutex.lock();
    if(samples*_channels*2 > (wrIndex-rdIndex))
            samples=(wrIndex-rdIndex)/(2*_channels);
    for(int i=0;i<6;i++) f[i]=0;
    if(!samples)
    {
        mutex.unlock();
        return true;
    }
#define USE_MEAN_SQUARE
    uint8_t *base8=rdIndex+audioBuffer;
    int16_t *base=(int16_t *)(base8);
    for(int i=0;i<samples;i++)
        for(int chan=0;chan<_channels;chan++)
        {
#ifdef USE_MEAN_SQUARE
                f[chan]+=base[0]*base[0];
#else
                f[chan]+=abs(*base);
#endif
                base++;
        }
    mutex.unlock();
    // Normalize
    for(int i=0;i<6;i++)
    {
        float d=f[i];
        d/=samples; // d is now between 0 and 32768
#ifdef USE_MEAN_SQUARE
        d=sqrt(d);
#endif
        d/=128;     // D is now between 0 and 255
        if(d>255) d=255;
        raw[i]=(uint32_t)d;
    }
    
    // Move channels around so that they fit Left/Right/Center/Rear Left, Rear Right,LFE
    const CHANNEL_TYPE *chans=this->getWantedChannelMapping(_channels);
    static const CHANNEL_TYPE output[6]={ADM_CH_FRONT_LEFT,ADM_CH_FRONT_CENTER,ADM_CH_FRONT_RIGHT,
                                         ADM_CH_REAR_LEFT,ADM_CH_REAR_RIGHT,ADM_CH_LFE};
    for(int i=0;i<6;i++)
    {
        CHANNEL_TYPE wanted=output[i];
        for(int j=0;j<_channels;j++)
        {
            if(chans[j]==wanted)
            {
                vol[i]=raw[j];
                break;
            }
        }
    }
#if 0
    #define zz vol
    printf("[%d] %02d %02d %02d %02d %02d %02d\n",samples,zz[0],zz[1],zz[2],zz[3],zz[4],zz[5]);
#endif
    return true;

}
//**
