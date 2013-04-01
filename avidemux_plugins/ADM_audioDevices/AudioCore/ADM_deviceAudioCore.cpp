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
ADM_DECLARE_AUDIODEVICE(CoreAudio,coreAudioDevice,1,0,2,"PulseAudioSimple audio device (c) mean");

static admMutex mutex;
static Component comp = NULL;
static int16_t audioBuffer[BUFFER_SIZE];
static AudioUnit theOutputUnit;
static uint32_t rd_ptr = 0;
static uint32_t wr_ptr = 0;

static OSStatus MyRenderer(void *inRefCon, AudioUnitRenderActionFlags *inActionFlags,
	const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData);
static OSStatus OverloadListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput,
	AudioDevicePropertyID inPropertyID, void* inClientData);
/**

*/
OSStatus OverloadListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput,
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

	return 1;
}
/**
*/
OSStatus MyRenderer(void *inRefCon, AudioUnitRenderActionFlags *inActionFlags, const AudioTimeStamp *inTimeStamp,
	UInt32 inBusNumber, UInt32 inChannel, AudioBufferList *ioData)
{
	uint32_t nb_sample = ioData->mBuffers[0].mDataByteSize >> 1;
	uint32_t left = 0;
	uint8_t *in, *out;
        mutex.lock();
	in = (uint8_t*)&audioBuffer[rd_ptr];
	out = (uint8_t*)ioData->mBuffers[0].mData;
	aprintf("[CoreAudio] Fill: rd %lu, wr %lu, nb asked %lu\n", rd_ptr, wr_ptr, nb_sample);

	if(wr_ptr>rd_ptr)
	{
		left=wr_ptr-rd_ptr-1;

		if(left>nb_sample)
		{
			memcpy(out,in,nb_sample*2);
			rd_ptr+=nb_sample;
		}

		else
		{
			memcpy(out,in,left*2);
			memset(out+left*2,0,(nb_sample-left)*2);
			rd_ptr+=left;
		}
	}
	else
	{
		// wrap
		left=BUFFER_SIZE-rd_ptr-1;
		if(left>nb_sample)
		{
			memcpy(out,in,nb_sample*2);
			rd_ptr+=nb_sample;
		}
		else
		{
			memcpy(out,in,left*2);
			out+=left*2;
			rd_ptr=0;
			in=(uint8_t *)&audioBuffer[0];
			nb_sample-=left;
			if(nb_sample>wr_ptr-1) nb_sample=wr_ptr-1;
			memcpy(out,in,nb_sample*2);
			rd_ptr=nb_sample;	
		}
	}
        mutex.unlock();
	return 0;
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
	input.inputProcRefCon = NULL;
	
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

void coreAudioDevice::sendData()
 {
 	// First put stuff into the buffer
	uint8_t *src;
	uint32_t left;

        mutex.lock();
        uint32_t avail=wrIndex-rdIndex;
        if(!avail)
        {
                mutex.unlock();
                // send silence
                //pa_simple_write(INSTANCE,silence, sizeOf10ms,&er);
        
                return ;
        }
 

	// We have room left, copy it
        uint8_t *data=audioBuffer+rdIndex;
        int len=avail/2;
	src=(uint8_t *)&audioBuffer[wr_ptr];

		memcpy(src,data,len*2);
		rdIndex+=len*2;
	//aprintf("AudioCore: Putting %lu bytes rd:%lu wr:%lu \n",len*2,rd_ptr,wr_ptr);
	mutex.unlock();	

	_inUse=1;
	verify_noerr(AudioOutputUnitStart(theOutputUnit));

	return ;
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



