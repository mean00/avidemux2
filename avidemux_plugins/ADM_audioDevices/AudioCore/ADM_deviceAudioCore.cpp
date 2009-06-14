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
#include "config.h"

#ifdef __APPLE__
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <AudioUnit/AudioUnit.h>

#include "ADM_default.h"
#include "ADM_assert.h"
#include "ADM_audiodevice.h"
#include "ADM_audiodevice/ADM_deviceAudioCore.h"
#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME  MODULE_ADEVICE
#include "ADM_osSupport/ADM_debug.h"

#define BUFFER_SIZE (500*48000)

static Component comp = NULL;
static int16_t audioBuffer[BUFFER_SIZE];
static AudioUnit theOutputUnit;
static uint32_t rd_ptr = 0;
static uint32_t wr_ptr = 0;
static pthread_mutex_t lock;

static OSStatus MyRenderer(void *inRefCon, AudioUnitRenderActionFlags *inActionFlags,
	const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData);
static OSStatus OverloadListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput,
	AudioDevicePropertyID inPropertyID, void* inClientData);

OSStatus OverloadListenerProc(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput,
	AudioDevicePropertyID inPropertyID, void* inClientData)
{
	printf ("[CoreAudio] *** Overload detected on device playing audio ***\n");
	return noErr;
}

uint8_t coreAudioDevice::setVolume(int volume) {}

coreAudioDevice::coreAudioDevice(void) 
{
	printf("[CoreAudio] Creating CoreAudio device\n");
	_inUse=0;
	pthread_mutex_init(&lock, NULL);
	pthread_mutex_unlock(&lock);
}

uint8_t coreAudioDevice::stop(void) 
{
	if (_inUse)
		verify_noerr(AudioOutputUnitStop(theOutputUnit));

	// Clean up
	CloseComponent(theOutputUnit);
	_inUse=0;

	return 1;
}

OSStatus MyRenderer(void *inRefCon, AudioUnitRenderActionFlags *inActionFlags, const AudioTimeStamp *inTimeStamp,
	UInt32 inBusNumber, UInt32 inChannel, AudioBufferList *ioData)
{
	pthread_mutex_lock(&lock);
	uint32_t nb_sample = ioData->mBuffers[0].mDataByteSize >> 1;
	uint32_t left = 0;
	uint8_t *in, *out;

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

	pthread_mutex_unlock(&lock);
	return 0;
}

#define CHECK_RESULT(msg) \
    if (err != noErr) \
	{ \
		printf("[CoreAudio] Failed to initialise CoreAudio: " msg "\n"); \
        return 0; \
    }

uint8_t coreAudioDevice::init(uint8_t channels, uint32_t fq) 
{
	_channels = channels;

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
		printf("[CoreAudio] Failed to find component\n");
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

	streamFormat.mSampleRate = fq;
	streamFormat.mFormatID = kAudioFormatLinearPCM;
	streamFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;

	streamFormat.mBytesPerPacket = channels * sizeof (UInt16);
	streamFormat.mFramesPerPacket = 1;
	streamFormat.mBytesPerFrame = channels * sizeof (UInt16);
	streamFormat.mChannelsPerFrame = channels;
	streamFormat.mBitsPerChannel = sizeof (UInt16) * 8;
	
	err = AudioUnitSetProperty(theOutputUnit,
		kAudioUnitProperty_StreamFormat,
		kAudioUnitScope_Input,
		0,
		&streamFormat,
		sizeof(streamFormat));
	CHECK_RESULT("AudioUnitSetProperty [StreamFormat]")
	
	printf("[CoreAudio] Rendering source:\n");
	printf("[CoreAudio] \tSampleRate = %f,\n", streamFormat.mSampleRate);
	printf("[CoreAudio] \tBytesPerPacket = %ld,\n", streamFormat.mBytesPerPacket);
	printf("[CoreAudio] \tFramesPerPacket = %ld,\n", streamFormat.mFramesPerPacket);
	printf("[CoreAudio] \tBytesPerFrame = %ld,\n", streamFormat.mBytesPerFrame);
	printf("[CoreAudio] \tBitsPerChannel = %ld,\n", streamFormat.mBitsPerChannel);
	printf("[CoreAudio] \tChannelsPerFrame = %ld\n", streamFormat.mChannelsPerFrame);

    return 1;
}

uint8_t coreAudioDevice::play(uint32_t len, float *data)
 {
 	// First put stuff into the buffer
	uint8_t *src;
	uint32_t left;

	dither16(data, len, _channels);

	pthread_mutex_lock(&lock);

	// Check we have room left
	if(wr_ptr>=rd_ptr)
		left=BUFFER_SIZE-(wr_ptr-rd_ptr);
	else
		left=rd_ptr-wr_ptr;

	if(len+1>left)
	{
		printf("[CoreAudio] Buffer full!\n");
		pthread_mutex_unlock(&lock);
		return 0;
	}

	// We have room left, copy it
	src=(uint8_t *)&audioBuffer[wr_ptr];

	if(wr_ptr+len<BUFFER_SIZE)
	{
		memcpy(src,data,len*2);
		wr_ptr+=len;
	}
	else
	{
		left=BUFFER_SIZE-wr_ptr-1;
		memcpy(src,data,left*2);
		memcpy(audioBuffer,data+left*2,(len-left)*2);
		wr_ptr=len-left;	
	}
	//aprintf("AudioCore: Putting %lu bytes rd:%lu wr:%lu \n",len*2,rd_ptr,wr_ptr);
	pthread_mutex_unlock(&lock);	

	_inUse=1;
	verify_noerr(AudioOutputUnitStart(theOutputUnit));

	return 1;
}
#else
void dummy_ac_func(void);
void dummy_ac_func(void)
{
}
#endif
