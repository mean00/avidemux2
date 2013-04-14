/***************************************************************************
                          ADM_deviceAudioCore.cpp  -  description
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
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#include <AudioUnit/AudioUnit.h>
#include "ADM_default.h"
#include "ADM_audiodevice.h"
#include "ADM_audioDeviceInternal.h"
#include "ADM_deviceAudioCore.h"
#define BUFFER_SIZE (500*48000)
#define aprintf(...) {}


ADM_DECLARE_AUDIODEVICE(CoreAudio,coreAudioDevice,1,0,2,"Core Audio Plugin  (c) mean");

/**

*/
static OSStatus OverloadListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput,
	AudioDevicePropertyID inPropertyID, void* inClientData)
{
	ADM_info ("[CoreAudio] *** Overload detected on device playing audio ***\n");
	return noErr;
}
/**

*/
uint8_t coreAudioDevice::setVolume(int volume) 
{
        return 1;
}
/**

*/
uint32_t coreAudioDevice::getLatencyMs(void)
{
        return 0;
}
/**

*/
coreAudioDevice::coreAudioDevice(void) 
{
	ADM_info("[CoreAudio] Creating CoreAudio device\n");
	_inUse=0;
        comp=NULL;
}
/**

*/
coreAudioDevice::~coreAudioDevice() 
{
	ADM_info("[CoreAudio]  destroying\n");
}
/**
*/
bool coreAudioDevice::localStop(void) 
{
	if (_inUse)
		verify_noerr(AudioOutputUnitStop(theOutputUnit));

	// Clean up
	CloseComponent(theOutputUnit);
	_inUse=0;
        ADM_usleep(10*1000);
	return 1;
}
/**
*/
OSStatus coreAudioDevice::MyRenderer(void *inRefCon, AudioUnitRenderActionFlags *inActionFlags, const AudioTimeStamp *inTimeStamp,
	UInt32 inBusNumber, UInt32 inChannel, AudioBufferList *ioData)
{
	uint32_t nb_sample = ioData->mBuffers[0].mDataByteSize >> 1;
	uint8_t *out = (uint8_t*)ioData->mBuffers[0].mData;
        coreAudioDevice *me=(coreAudioDevice *)inRefCon;
        me->sendMoreData(nb_sample,out);
        return 0;
}
/**

*/
bool coreAudioDevice::sendMoreData(int nbSample, uint8_t *where)
{
        if(!_inUse) return false;
        mutex.lock();

        uint32_t avail=(wrIndex-rdIndex)>>1;
        int filler=0;
        if(nbSample<avail) avail=nbSample;
        if(nbSample>avail) filler=nbSample-avail;
        //printf("Audio : avail =%d samples,requested=%d,filler=%d\n",avail,nbSample,filler);
	uint8_t *in;
	in = (uint8_t*)audioBuffer.at(rdIndex);
        if(avail)
                memcpy(where,in,avail*2);
        if(filler)
                memset(where+avail*2,0,filler*2);
        rdIndex+=avail*2;

        mutex.unlock();
	return 0;
}

void coreAudioDevice::sendData()
 {
        if(!_inUse)
        {
	        _inUse=1;
	        verify_noerr(AudioOutputUnitStart(theOutputUnit));
        }
  again:
        mutex.lock();
        int avail=wrIndex-rdIndex;
        if(avail>sizeOf10ms*5) // buffer filling up, sleep a bit
        {
                mutex.unlock();
                ADM_usleep(20*1000);
                goto again;
        }
        mutex.unlock();
	return ;
}


#define CHECK_RESULT(msg) \
    if (err != noErr) \
	{ \
		ADM_info("[CoreAudio] Failed to initialise CoreAudio: " msg "\n"); \
        return 0; \
    }
/**
        \fn localInit
*/
bool coreAudioDevice::localInit(void) 
{

	OSStatus err;
	ComponentDescription desc;
	AURenderCallbackStruct input;
	AudioStreamBasicDescription streamFormat;
	AudioDeviceID theDevice;

	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_HALOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;

	comp = FindNextComponent(NULL, &desc);

	if (comp == NULL)
	{
		ADM_info("[CoreAudio] Failed to find component\n");
		return 0;
	}

	err = OpenAComponent(comp, &theOutputUnit);
	CHECK_RESULT("OpenAComponent")

	err = AudioUnitInitialize(theOutputUnit);
	CHECK_RESULT("AudioUnitInitialize")
	
	// Set up a callback function to generate output to the output unit
	input.inputProc = MyRenderer;
	input.inputProcRefCon = this;
	
	err = AudioUnitSetProperty(theOutputUnit, 
					kAudioUnitProperty_SetRenderCallback,
					kAudioUnitScope_Global,
					0,
					&input, 
					sizeof(input));
	CHECK_RESULT("AudioUnitSetProperty [SetInputCallback]")

	streamFormat.mSampleRate = _frequency;
	streamFormat.mFormatID = kAudioFormatLinearPCM;
	streamFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;

	streamFormat.mBytesPerPacket = _channels * sizeof (UInt16);
	streamFormat.mFramesPerPacket = 1;
	streamFormat.mBytesPerFrame = _channels * sizeof (UInt16);
	streamFormat.mChannelsPerFrame = _channels;
	streamFormat.mBitsPerChannel = sizeof (UInt16) * 8;
	
	err = AudioUnitSetProperty(theOutputUnit,
		kAudioUnitProperty_StreamFormat,
		kAudioUnitScope_Input,
		0,
		&streamFormat,
		sizeof(streamFormat));
	CHECK_RESULT("AudioUnitSetProperty [StreamFormat]")
	
	ADM_info("[CoreAudio] Rendering source:\n");
	ADM_info("[CoreAudio] \tSampleRate = %f,\n", streamFormat.mSampleRate);
	ADM_info("[CoreAudio] \tBytesPerPacket = %ld,\n", streamFormat.mBytesPerPacket);
	ADM_info("[CoreAudio] \tFramesPerPacket = %ld,\n", streamFormat.mFramesPerPacket);
	ADM_info("[CoreAudio] \tBytesPerFrame = %ld,\n", streamFormat.mBytesPerFrame);
	ADM_info("[CoreAudio] \tBitsPerChannel = %ld,\n", streamFormat.mBitsPerChannel);
	ADM_info("[CoreAudio] \tChannelsPerFrame = %ld\n", streamFormat.mChannelsPerFrame);

    return 1;
}
/**
    \fn getWantedChannelMapping
*/
static const CHANNEL_TYPE mono[MAX_CHANNELS]={ADM_CH_MONO};
static const CHANNEL_TYPE stereo[MAX_CHANNELS]={ADM_CH_FRONT_LEFT,ADM_CH_FRONT_RIGHT};
static const CHANNEL_TYPE fiveDotOne[MAX_CHANNELS]={ADM_CH_FRONT_LEFT,ADM_CH_FRONT_RIGHT,ADM_CH_FRONT_CENTER,
                                             ADM_CH_REAR_LEFT,ADM_CH_REAR_RIGHT,ADM_CH_LFE};
/**
*/
const CHANNEL_TYPE *coreAudioDevice::getWantedChannelMapping(uint32_t channels)
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



