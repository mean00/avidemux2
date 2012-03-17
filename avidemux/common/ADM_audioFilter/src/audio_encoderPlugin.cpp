/***************************************************************************
                          audio_encoderPlugin.cpp  -  description
                             -------------------
    
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
#include <BVector.h>
#include "ADM_default.h"
#include "audioencoderInternal.h"
#include "ADM_dynamicLoading.h"


static AUDIOENCODER  currentEncoder=0; //0 is always dummy

static AUDIOENCODER ADM_encoderByName(const char *name);


/**
    \class ADM_AudioEncoderLoader
    \brief Helper class to load plugins
*/
class ADM_AudioEncoderLoader :public ADM_LibWrapper
{

public:
        int                 initialised;
        ADM_audioEncoder    *encoderBlock;
        


        ADM_AudioEncoderLoader(const char *file) : ADM_LibWrapper()
		{
                ADM_audioEncoder    *e;
                ADM_audioEncoder *(*getInfo)(void);
                initialised = loadLibrary(file) && getSymbols(1,
				&getInfo, "getInfo"
                );
                encoderBlock=NULL;
                if(initialised)
                {
                    e=getInfo();
                    if(e->apiVersion!=ADM_AUDIO_ENCODER_API_VERSION)
                    {
                        e=NULL;
                        initialised=0;
                    }else
                    {
                        printf("[AudioEncoder] Loaded %s version %02d.%02d.%02d wavTag :0x%x\n",e->codecName,
                                e->major,e->minor,e->patch,e->wavTag);
                        encoderBlock=new ADM_audioEncoder;
                        *encoderBlock=*e;
                        encoderBlock->opaque=(void *)this;
                    } 
                }else
                {
                    printf("Symbol loading failed for %s\n",file);
                }
		}
        ADM_AudioEncoderLoader(const char *name, const char *menuName) : ADM_LibWrapper()
		{
                    initialised=false;
                    encoderBlock=new ADM_audioEncoder;
                    memset(encoderBlock,0,sizeof(*encoderBlock));
                    encoderBlock->codecName=name;
                    encoderBlock->menuName=menuName;                    
                    encoderBlock->opaque=(void *)this;
		}
        ~ADM_AudioEncoderLoader()
        {
            if(encoderBlock) delete encoderBlock;
            encoderBlock=NULL;
            
        }
};
static BVector <ADM_audioEncoder *> ListOfAudioEncoder;
static BVector <ADM_AudioEncoderLoader *> ListOfAudioEncoderLoader;

/**
        \fn ADM_ae_getPluginNbEncoders
        \brief Returns the number of av filter plugins except one
*/
uint32_t ADM_ae_getPluginNbEncoders(void)
{
    return ListOfAudioEncoder.size()-1;
}
/**
    \fn     ADM_ae_getAPluginEncoderInfo
    \brief  Get Infos about the encoder #th plugin (plugin display)
*/
bool     ADM_ae_getAPluginEncoderInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch)
{
    filter++;
    ADM_assert(filter<ListOfAudioEncoder.size());
    *major=ListOfAudioEncoder[filter]->major;
    *minor=ListOfAudioEncoder[filter]->minor;
    *patch=ListOfAudioEncoder[filter]->patch;
    *name=ListOfAudioEncoder[filter]->description;
    return true;
}
/**
    \fn tryLoadingFilterPlugin
    \brief Try loading the file given as argument as an audio device plugin

*/
#define Fail(x) {printf("%s:"#x"\n",file);goto er;}
static bool tryLoadingFilterPlugin(const char *file)
{
	ADM_AudioEncoderLoader *dll=new ADM_AudioEncoderLoader(file);
    if(!dll->initialised) Fail(CannotLoad);
    
    ListOfAudioEncoderLoader.append(dll);
    ListOfAudioEncoder.append(dll->encoderBlock);  // will be destroyed when Loader is destroyed
    printf("[AudioEncoder] Registered filter %s as  %s\n",file,dll->encoderBlock->description);
    return true;
	// Fail!
er:
	delete dll;
	return false;

}
/**
 * 	\fn ADM_ae_loadPlugins
 *  \brief load all audio encoder plugins
 */
uint8_t ADM_ae_loadPlugins(const char *path)
{
#define MAX_EXTERNAL_FILTER 100
// FIXME Factorize

	char *files[MAX_EXTERNAL_FILTER];
	uint32_t nbFile;
    // Add the copy encoder
    ADM_AudioEncoderLoader *copy=new ADM_AudioEncoderLoader("copy","Copy");
    ListOfAudioEncoder.append(copy->encoderBlock);
    //
	memset(files,0,sizeof(char *)*MAX_EXTERNAL_FILTER);
	printf("[ADM_ae_plugin] Scanning directory %s\n",path);

	if(!buildDirectoryContent(&nbFile, path, files, MAX_EXTERNAL_FILTER, SHARED_LIB_EXT))
	{
		printf("[ADM_ae_plugin] Cannot parse plugin\n");
		return 0;
	}

	for(int i=0;i<nbFile;i++)
		tryLoadingFilterPlugin(files[i]);

	printf("[ADM_ae_plugin] Scanning done\n");
    int nb=ListOfAudioEncoder.size();
    for(int i=1;i<nb;i++)
        for(int j=i+1;j<nb;j++)
        {
             ADM_audioEncoder *a,*b;
             a=ListOfAudioEncoder[i];
             b=ListOfAudioEncoder[j];
             if(strcmp(a->menuName,b->menuName)>0)
             {
                ListOfAudioEncoder[j]=a;
                ListOfAudioEncoder[i]=b;
             }
        }
        clearDirectoryContent(nbFile,files);
	return 1;
}
/**
    \fn ADM_ae_cleanup
*/
bool ADM_ae_cleanup(void)
{
    for(uint32_t i=0;i<ListOfAudioEncoderLoader.size();i++)
	{
		ADM_AudioEncoderLoader *a=ListOfAudioEncoderLoader[i];
        delete a;
        ListOfAudioEncoderLoader[i]=NULL;
	}
    
    ListOfAudioEncoderLoader.clear();
    return true;
}
/**
    \fn audioPrintCurrentCodec
    \brief updates the UI with the current selected audio encoder
*/
void UI_setAudioCodec( int i);
void audioPrintCurrentCodec(void)
{
			UI_setAudioCodec(currentEncoder);
}

/**
    \fn ADM_encoderByName
    \brief Returns the Id of the given string 

*/
AUDIOENCODER ADM_encoderByName(const char *name)
{
	if(!name) return (AUDIOENCODER)0;
    if(!strcasecmp(name,"copy")) return (AUDIOENCODER)0; // copy
	for(uint32_t i=1;i<ListOfAudioEncoder.size();i++)
	{
		if(!strcasecmp(name,ListOfAudioEncoder[i]->codecName))
		{
			return i;
		}	
	}
	printf("[AudioEncoder] Encoder not found :%s\n",name);
	return (AUDIOENCODER)0;

}
/**
    \fn    ADM_audioEncoderById
    \brief Returns the name of a device from its Id
*/
static const char *ADM_audioEncoderById(AUDIOENCODER id)
{

	ADM_assert(id<ListOfAudioEncoder.size());
    return ListOfAudioEncoder[id]->codecName;
}
/**
    \fn AVDM_getCurrentAudioEncoder
    \brief
*/
AUDIOENCODER AVDM_getCurrentAudioEncoder( void)
{
	return currentEncoder;
}
/**
    \fn audioCodecSelect
    \brief Update UI
*/
uint8_t DIA_audioCodec( int *codec );
void audioCodecSelect( void )
{
#warning FIXME 
#warning FIXME 
#warning FIXME 
	//DIA_audioCodec( &currentEncoder );
	audioPrintCurrentCodec();
}
/**
    \fn     audioCodecSetByName
    \brief  only called by JS, we have to update UI as well
*/
uint8_t audioCodecSetByName( const char *name)
{
		for(uint32_t i=0;i<ListOfAudioEncoder.size();i++)
		{
			if(!strcasecmp(name,ListOfAudioEncoder[i]->codecName))
			{

				currentEncoder=i;
                audioPrintCurrentCodec(); // Update UI
				return 1;
			}

		}
		printf("\n Mmmm Select audio codec by name failed...(%s).\n",name);
		return 0;
}
/**
    \fn audioCodecSetByIndex
    \brief To be used by UI code only!
*/
uint8_t audioCodecSetByIndex(int i)
{
    ADM_assert(i<ListOfAudioEncoder.size());
    currentEncoder=i;
    printf("[AudioEncoder] Selected %s for index %d, tag 0x%x \n",ListOfAudioEncoder[currentEncoder]->codecName,i,ListOfAudioEncoder[currentEncoder]->wavTag);
    return 1;

}
/**
    \fn audioCodecGetName
    \brief Returns the current codec tagname
*/
const char *audioCodecGetName( void )
{
	  ADM_assert(currentEncoder<ListOfAudioEncoder.size());
      return ListOfAudioEncoder[currentEncoder]->codecName;

}

/**
    \fn audioProcessMode
    \brief
    @return 1 in process mode, 0 in copy mode
*/
uint32_t audioProcessMode(void)
{
        if(!currentEncoder) return 0;
        return 1;
}
/**
 * 	\fn getAudioOuputTag
 *  \brief Return the encoding of the currently selected codec
 *  Must be called only in process mode, else it is meaningless.
 */
uint32_t audioFilter_getOuputCodec(void)
{
	ADM_assert(!currentEncoder);
    return ListOfAudioEncoder[currentEncoder]->wavTag;
}

/**
 * 	\fn audioFilter_getMaxChannels
 *  \brief Return the max # of channels a codec supports
 */
uint32_t audioFilter_getMaxChannels(void)
{
    ADM_assert(currentEncoder<ListOfAudioEncoder.size());
	if(!currentEncoder) return 99999;
	return ListOfAudioEncoder[currentEncoder]->maxChannels;
}
/**
    \fn audioCodecConfigure
    \brief
*/
void audioCodecConfigure( void )
{
    if(ListOfAudioEncoder[currentEncoder]->configure)
    ListOfAudioEncoder[currentEncoder]->configure();
}
/**
    \fn audioGetBitrate
*/
uint32_t audioGetBitrate(void)
{
    ADM_assert(currentEncoder<ListOfAudioEncoder.size());
    if(ListOfAudioEncoder[currentEncoder]->getBitrate)
        return ListOfAudioEncoder[currentEncoder]->getBitrate();
    return 0;
} 
void audioFilter_SetBitrate( int i)
{
    ADM_assert(currentEncoder<ListOfAudioEncoder.size());
    if(ListOfAudioEncoder[currentEncoder]->setBitrate)
        ListOfAudioEncoder[currentEncoder]->setBitrate(i);
    
}
/**
    \fn audioEncoderGetNumberOfEncoders
*/
uint32_t audioEncoderGetNumberOfEncoders(void)
{
    return ListOfAudioEncoder.size();
}
/**
    \fn audioEncoderGetDisplayName
*/
const char  *audioEncoderGetDisplayName(uint32_t i)
{
     ADM_assert(currentEncoder<ListOfAudioEncoder.size());
     return ListOfAudioEncoder[i]->menuName;
}
/**
        \fn audioEncoderCreate
        \brief Spawn an audio encoder
*/
ADM_AudioEncoder *audioEncoderCreate(AUDMAudioFilter *filter,bool globalHeader)
{
      ADM_assert(currentEncoder<ListOfAudioEncoder.size());
      ADM_audioEncoder *enc=ListOfAudioEncoder[currentEncoder];
      return enc->create(filter,globalHeader);
}
/**
        \fn getAudioExtraConf
        \brief 
*/

bool getAudioExtraConf(uint32_t *bitrate,CONFcouple **couple)
{
    if(!currentEncoder)
    {
        printf("[AudioEncoder] Cannot get conf on copy!\n");
        return 0;
    }
     ADM_assert(currentEncoder<ListOfAudioEncoder.size());
     ADM_audioEncoder *encoder= ListOfAudioEncoder[currentEncoder];
     if(encoder->getBitrate)
        *bitrate=encoder->getBitrate();
     else
        *bitrate=128; // PCM does not have global conf bitrate
     if(encoder->getConfigurationData)
        return encoder->getConfigurationData(couple);
     else return 1;
}
/**
    \fn setAudioExtraConf
*/
bool setAudioExtraConf(uint32_t bitrate,CONFcouple *c)
{
    if(!currentEncoder)
    {
        printf("[AudioEncoder] Cannot set conf on copy!\n");
        return false;
    }
     ADM_assert(currentEncoder<ListOfAudioEncoder.size());
     ADM_audioEncoder *encoder= ListOfAudioEncoder[currentEncoder];
     if(encoder->setBitrate)
        encoder->setBitrate(bitrate);
     if(encoder->setConfigurationData)
     {
        if(c)             return encoder->setConfigurationData(c);
        ADM_warning("Null extra configuration given to audio codec!\n");
     }
     return true;
}     
/**
        \fn audio_selectCodecByTag
        \brief Select the "best" encoder outputing tag codec
*/
uint8_t audio_selectCodecByTag(uint32_t tag)
{
    int selected=-1,priority=-1;
    for(int i=1;i<ListOfAudioEncoder.size();i++)
    {
        ADM_audioEncoder *c=ListOfAudioEncoder[i];
        if(c->wavTag==tag)
        {
            if((int)c->priority>priority)
            {
                selected=i;
                priority=c->priority;
            }
        }
    }
    if(selected!=-1)
    {
        currentEncoder=selected;
        UI_setAudioCodec( (int)currentEncoder);
        printf("[AudioEncoder] Selected %s for tag %d (%s)\n",ListOfAudioEncoder[currentEncoder]->codecName,tag,"");
        return 1;
    }
    return 0;
}
/**
         \fn audioSetOption
         \brief Allow per codec switch
*/
uint8_t audioSetOption(const char *option, uint32_t value)
{
    ADM_audioEncoder *c=ListOfAudioEncoder[currentEncoder];
    if(!c->setOption) return 0;
    return c->setOption(option,value);

}
/**
         \fn audio_setCopyCodec
         \brief Set audio codec to copy
*/
uint8_t audio_setCopyCodec(void)
{
    currentEncoder=0;
    return 1;

}

//**
