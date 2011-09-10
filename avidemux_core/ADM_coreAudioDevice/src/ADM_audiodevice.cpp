/***************************************************************************
                          ADM_audiodevice.cpp  -  description
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
#include <BVector.h>
#include "ADM_default.h"
#include "ADM_audiodevice.h"
#include "audio_out.h"
#include "ADM_audioDeviceInternal.h"
#include "prefs.h"
#include "ADM_dynamicLoading.h"
BVector <ADM_AudioDevices *> ListOfAudioDevices;


static audioDeviceThreaded *device=NULL;
static AUDIO_DEVICE  currentDevice=0; //0 is always dummy

static AUDIO_DEVICE ADM_audioByName(const char *name);
static const char *ADM_audioById(AUDIO_DEVICE id);


// --------- couple of stubs for dummy device  -------------
static uint8_t      DummyGetVersion(uint32_t *major,uint32_t *minor,uint32_t *patch)
{
    *major=1;
    *minor=0;
    *patch=0;
    return 0;
}
audioDeviceThreaded *DummyCreateAudioDevice(void)
{
    return new dummyAudioDevice;
}
void DummyDeleteAudioDevice(audioDeviceThreaded *z)
{
    dummyAudioDevice *a=(dummyAudioDevice *)z;
}
// --------- couple of stubs for dummy device  -------------
/**
        \fn ADM_av_getNbDevices
        \brief Returns the number of av filter plugins except one
*/
uint32_t ADM_av_getNbDevices(void)
{
    return ListOfAudioDevices.size()-1;
}
/**
    \fn     ADM_av_getDeviceInfo
    \brief  Get Infos about the filter#th plugin
*/
bool     ADM_av_getDeviceInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch)
{
    filter++;
    ADM_assert(filter<ListOfAudioDevices.size());
    ListOfAudioDevices[filter]->getVersion(major,minor,patch);
    *name=ListOfAudioDevices[filter]->name;
    return true;
}
/**
    \fn tryLoadingFilterPlugin
    \brief Try loading the file given as argument as an audio device plugin

*/
#define Fail(x) {printf("%s:"#x"\n",file);goto er;}
static bool tryLoadingFilterPlugin(const char *file)
{
	ADM_AudioDevices *dll=new ADM_AudioDevices(file);
    if(!dll->initialised) Fail(CannotLoad);
    if(dll->apiVersion!=ADM_AUDIO_DEVICE_API_VERSION) Fail(WrongApiVersion);

    ListOfAudioDevices.append(dll); // Needed for cleanup. FIXME TODO Delete it.
    printf("[Filters] Registered filter %s as  %s\n",file,dll->descriptor);
    return true;
	// Fail!
er:
	delete dll;
	return false;

}
/**
 * 	\fn ADM_av_loadPlugins
 *  \brief load all audio device plugins
 */
uint8_t ADM_av_loadPlugins(const char *path)
{
#define MAX_EXTERNAL_FILTER 100

	char *files[MAX_EXTERNAL_FILTER];
	uint32_t nbFile;


    // PushBack our dummy one : TODO FIXME
    ADM_AudioDevices *dummyDevice=new ADM_AudioDevices("Dummy","Dummy audio device", 
                                DummyGetVersion,
                                DummyCreateAudioDevice,
                                DummyDeleteAudioDevice);
    
    ListOfAudioDevices.append(dummyDevice); 
	memset(files,0,sizeof(char *)*MAX_EXTERNAL_FILTER);
	printf("[ADM_av_plugin] Scanning directory %s\n",path);

	if(!buildDirectoryContent(&nbFile, path, files, MAX_EXTERNAL_FILTER, SHARED_LIB_EXT))
	{
		printf("[ADM_av_plugin] Cannot parse plugin\n");
		return 0;
	}

	for(int i=0;i<nbFile;i++)
		tryLoadingFilterPlugin(files[i]);

	printf("[ADM_av_plugin] Scanning done\n");
        clearDirectoryContent(nbFile,files);

	return 1;
}
/**
    \fn AVDM_audioSave
    \brief Save in Prefs the current audio Device

*/

void AVDM_audioSave( void )
{
const char *string;
		string=ADM_audioById(currentDevice);
		prefs->set(AUDIO_DEVICE_AUDIODEVICE, string);
}
/**
    \fn ADM_audioByName
    \brief Returns the Id of the given string 

*/
AUDIO_DEVICE ADM_audioByName(const char *name)
{
	if(!name) return (AUDIO_DEVICE)0;
	for(uint32_t i=0;i<ListOfAudioDevices.size();i++)
	{
		if(!strcasecmp(name,ListOfAudioDevices[i]->name))
		{
			return i;
		}	
	}
	printf("Device not found :%s\n",name);
	return (AUDIO_DEVICE)0;

}
/**
    \fn ADM_audioById
    \brief Returns the name of a device from its Id
*/
const char *ADM_audioById(AUDIO_DEVICE id)
{
	ADM_assert(id<ListOfAudioDevices.size());
    return ListOfAudioDevices[id]->name;
}
/**
    \fn AVDM_getCurrentDevice
    \brief
*/
AUDIO_DEVICE AVDM_getCurrentDevice( void)
{
	return currentDevice;
}
/**
    \fn AVDM_audioInit
    \brief
*/
void AVDM_audioInit(void )
{
uint8_t init=0;
char *name=NULL;
AUDIO_DEVICE id=0;

		if(prefs->get(AUDIO_DEVICE_AUDIODEVICE, &name))
		{
		id=ADM_audioByName(name);
		ADM_dealloc(name);
		name=NULL;	
        }
		
		
        AVDM_switch(id);
}
/**
        \fn AVDM_cleanup
        \brief Current device is no longer used, delete
*/
void AVDM_cleanup(void)
{
    int nb=ListOfAudioDevices.size();
    for(int i=0;i<nb;i++)
            delete ListOfAudioDevices[i];
    ListOfAudioDevices.clear();
	if(device)
	{
		delete device;
		device=NULL;
	}
}
/**
    \fn AVDM_switch
    \brief Change audio device
*/
void AVDM_switch(AUDIO_DEVICE action)
{
	if(device)
	{
		delete device;
		device=NULL;
	}
    ADM_assert(action<ListOfAudioDevices.size());
    device=ListOfAudioDevices[action]->createAudioDevice();
    currentDevice=action;

}
/**
    \fn AVDM_AudioClose
    \brief Stop playback

*/
void AVDM_AudioClose(void)
{
	device->stop();
}

/**
    \fn AVDM_AudioSetup
    \brief Initialize a device

*/
uint32_t AVDM_AudioSetup(uint32_t fq, uint8_t channel,CHANNEL_TYPE *channelMapping)
{
	
	return device->init(channel,fq,channelMapping);
}
/**
    \fn AVDM_setVolume
    \brief Set the volume (0..100)

*/

uint8_t         AVDM_setVolume(int volume)
{
        printf("New volume :%d\n",volume);
        device->setVolume(volume);
        return 1;

}
/**
    \fn AVDM_AudioPlay
    \brief Send float data to be played immediately by the device

*/
uint8_t AVDM_AudioPlay(float *ptr, uint32_t nb)
{
	return device->play(nb,ptr);
}
/**
    \fn AVDM_GetLayencyMs
    \brief Return playback latency in ms

*/
uint32_t AVDM_GetLayencyMs(void)
{
	return device->getLatencyMs();
}

/**
    \fn AVDM_getMsFullness
    \brief returns the # of ms worth in the buffer
*/
uint32_t AVDM_getMsFullness(void)
{
    return device->getBufferFullness();

}
/**
    \fn AVDM_getStats
*/
bool        AVDM_getStats(uint32_t *vol)
{
    return device->getVolumeStats(vol);
}


//**
const CHANNEL_TYPE dummyAudioDevice::myChannelType[MAX_CHANNELS]=
                            {ADM_CH_FRONT_LEFT,ADM_CH_FRONT_RIGHT,ADM_CH_FRONT_CENTER,
                             ADM_CH_REAR_LEFT, ADM_CH_REAR_RIGHT, ADM_CH_LFE,
                             ADM_CH_INVALID,   ADM_CH_INVALID
                            };
bool dummyAudioDevice::localInit(void)                                {return true;}
bool dummyAudioDevice::localStop(void)                                {return true;}
void  dummyAudioDevice::sendData(void)                                {ADM_usleep(5000);}



// Else the linker will discard it...
#include "ADM_audioDeviceThreaded.cpp"
//**
