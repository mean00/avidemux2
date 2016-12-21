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
        std::string   name;
        std::string   descriptor;
        uint32_t      apiVersion;

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
                    name      =std::string(getAudioDeviceName());
                    apiVersion=            getApiVersion();
                    descriptor=std::string(getDescriptor());
                    ADM_info("Name :%s ApiVersion :%d\n",name.c_str(),apiVersion);
                }else
                {
                    ADM_warning("Symbol loading failed for %s\n",file);
                }
		}
        ADM_AudioDevices(const char *name,const char *desc, 
                                uint8_t      (*getVersion)(uint32_t *major,uint32_t *minor,uint32_t *patch),
                                audioDeviceThreaded *(*createAudioDevice)(),
                                void         (*deleteAudioDevice)(audioDeviceThreaded *device)) : ADM_LibWrapper()
		{

                    this->name=std::string(name);
                    this->descriptor=std::string(desc);
                    this->apiVersion=ADM_AUDIO_DEVICE_API_VERSION;
                    this->createAudioDevice=createAudioDevice;
                    this->deleteAudioDevice=deleteAudioDevice;
                    this->getVersion=getVersion;
                    
		}
      
};

#define ADM_DECLARE_AUDIODEVICE(name,Class,major,minor,patch,desc) \
extern "C" { \
ADM_PLUGIN_EXPORT const char *getName(void) {return #name;}\
ADM_PLUGIN_EXPORT uint32_t getApiVersion(void) {return ADM_AUDIO_DEVICE_API_VERSION;} \
ADM_PLUGIN_EXPORT const char *getDescriptor(void ) {return desc;} \
ADM_PLUGIN_EXPORT audioDeviceThreaded *create(void){return new Class;} \
ADM_PLUGIN_EXPORT void destroy(audioDeviceThreaded *z){Class *a=(Class *)z;delete a;} \
ADM_PLUGIN_EXPORT uint8_t getVersion(uint32_t *mmajor,uint32_t *mminor,uint32_t *ppatch) \
    {\
        *mmajor=major;\
        *mminor=minor;\
        *ppatch=patch;\
        return 1;\
    } \
}
#endif
