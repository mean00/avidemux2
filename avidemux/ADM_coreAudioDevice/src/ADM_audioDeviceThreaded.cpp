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

/**
    \fn audioDeviceThreaded
    \brief constructor
*/
audioDeviceThreaded::audioDeviceThreaded(void)
{

}
/**
    \fn audioDeviceThreaded
    \brief destructor
*/
audioDeviceThreaded::~audioDeviceThreaded()
{

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
    stopRequest=AUDIO_DEVICE_STOP_GR;
    printf("[AudioDeviceThreaded] Exiting loop\n");
}
/**
    \fn audioDeviceThreaded
    \brief destructor
*/
uint8_t audioDeviceThreaded::init(uint32_t channel, uint32_t fq )
{
    // Allocate buffer
    _channels=channel;
    _frequency=fq;
    sizeOf10ms=(_channels*_frequency*2)/100;
    sizeOf10ms&=~15; // make sure it is a multiple of 16
    silence=new uint8_t[sizeOf10ms];
    memset(silence,0,sizeOf10ms);
    audioBuffer=new uint8_t[ADM_THREAD_BUFFER_SIZE];
    rdIndex=wrIndex=0;
    stopRequest=AUDIO_DEVICE_STOPPED;
    //
    if(!localInit()) return 0;
    // Spawn
    stopRequest=AUDIO_DEVICE_STARTED;
    ADM_assert(!pthread_create(&myThread,NULL,bouncer,this));

    return 1;
}
/**
    \fn getBufferFullness
    \brief Returns the number of ms of audio in the buffer

*/
uint32_t   audioDeviceThreaded:: getBufferFullness(void)
{
    mutex.lock();
    float nbBytes=wrIndex-rdIndex;
    mutex.unlock();
    nbBytes/=10*sizeOf10ms;
    return 1+(uint32_t)nbBytes;
}
/**
    \fn stop
    \brief stop
*/
uint8_t audioDeviceThreaded::stop()
{
    if(audioBuffer)
    {
        delete [] audioBuffer;
        audioBuffer=NULL;
    }

    if(stopRequest==AUDIO_DEVICE_STARTED)
    {
        stopRequest=AUDIO_DEVICE_STOP_REQ;
        while(stopRequest==AUDIO_DEVICE_STOP_REQ)
        {
            ADM_usleep(1000);
        }
    }
    localStop();
    if(silence) delete [] silence;
    silence=NULL;
    stopRequest=AUDIO_DEVICE_STOPPED;
    return 1;
}
/**
    \fn write

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
	dither16(data, len, _channels);
    len*=2;
    return writeData((uint8_t *)data,len);

}

//**
