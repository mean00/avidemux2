/**
    \fn ADM_audioDeviceInternal.h
    \brief Macros and stuff to write audioDevice plugins



*/
#ifndef ADM_audioDeviceInternal_H
#define ADM_audioDeviceInternal_H

#define ADM_AUDIO_DEVICE_API_VERSION 2
#include "ADM_dynamicLoading.h"
class ADM_AudioDevices :public ADM_LibWrapper
{
public:
        int         initialised;
        audioDeviceThreaded *(*createAudioDevice)();
        void         (*deleteAudioDevice)(audioDeviceThreaded *device);
        uint8_t      (*getVersion)(uint32_t *major,uint32_t *minor,uint32_t *patch);
        // Only initialized once
        const char    *name;
        const char    *descriptor;
        uint32_t apiVersion;

        ADM_AudioDevices(const char *file) : ADM_LibWrapper()
		{
        const char   *(*getDescriptor)();
        uint32_t     (*getApiVersion)();
        const char  *(*getAudioDeviceName)();

			initialised = (loadLibrary(file) && getSymbols(6,
				&createAudioDevice, "create",
				&deleteAudioDevice, "destroy",

				&getAudioDeviceName, "getName",
				&getApiVersion, "getApiVersion",
				&getVersion, "getVersion",
				&getDescriptor, "getDescriptor"));
                if(initialised)
                {
                    name=getAudioDeviceName();
                    apiVersion=getApiVersion();
                    descriptor=getDescriptor();
                    printf("Name :%s ApiVersion :%d\n",name,apiVersion);
                }else
                {
                    printf("Symbol loading failed for %s\n",file);
                }
		}
        ADM_AudioDevices(const char *name,const char *desc, 
                                uint8_t      (*getVersion)(uint32_t *major,uint32_t *minor,uint32_t *patch),
                                audioDeviceThreaded *(*createAudioDevice)(),
                                void         (*deleteAudioDevice)(audioDeviceThreaded *device)) : ADM_LibWrapper()
		{

                    this->name=name;
                    this->descriptor=desc;
                    this->apiVersion=ADM_AUDIO_DEVICE_API_VERSION;
                    this->createAudioDevice=createAudioDevice;
                    this->deleteAudioDevice=deleteAudioDevice;
                    this->getVersion=getVersion;
                    
		}
      
};

#define ADM_DECLARE_AUDIODEVICE(name,Class,major,minor,patch,desc) \
extern "C" { \
const char *getName(void) {return #name;}\
uint32_t getApiVersion(void) {return ADM_AUDIO_DEVICE_API_VERSION;} \
const char *getDescriptor(void ) {return desc;} \
 audioDeviceThreaded *create(void){return new Class;} \
 void destroy(audioDeviceThreaded *z){Class *a=(Class *)z;delete a;} \
uint8_t getVersion(uint32_t *mmajor,uint32_t *mminor,uint32_t *ppatch) \
    {\
        *mmajor=major;\
        *mminor=minor;\
        *ppatch=patch;\
        return 1;\
    } \
}
#endif
