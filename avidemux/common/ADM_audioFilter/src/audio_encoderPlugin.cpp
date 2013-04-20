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

#include <string>
#include "BVector.h"
#include "ADM_default.h"
#include "audioencoderInternal.h"
#include "ADM_dynamicLoading.h"
#include "ADM_edit.hxx"
extern ADM_Composer *video_body;
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
    \fn audioEncoderGetNumberOfEncoders
*/
uint32_t audioEncoderGetNumberOfEncoders(void)
{
    return ListOfAudioEncoder.size();
}

/**
    \fn audioEncoderGetDisplayName
*/
const char  *audioEncoderGetDisplayName(int dex)
{
#if 0
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot set codec for track %d\n",dex);
        return "None";
    }
     int t=ed->encoderIndex;
     ADM_assert(t<ListOfAudioEncoder.size());
     return ListOfAudioEncoder[t]->menuName;
#endif
    if(dex>=ListOfAudioEncoder.size())
    {
        return "None";
    }
    return ListOfAudioEncoder[dex]->menuName;
}
// ********************************************************************
// ********************************************************************
// ********************************************************************
// ********************************************************************

/**
    \fn audioPrintCurrentCodec
    \brief updates the UI with the current selected audio encoder
*/
void UI_setAudioCodec( int i);
void audioPrintCurrentCodec(int dex)
{
        EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
        if(ed)
			UI_setAudioCodec(ed->encoderIndex);
}

/**
    \fn AVDM_getCurrentAudioEncoder
    \brief
*/
AUDIOENCODER AVDM_getCurrentAudioEncoder( int dex)
{
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(ed)
        return ed->encoderIndex;
    ADM_warning("Cant get current audio encoder for track %d\n",dex);
    return (AUDIOENCODER)0;
}
/**
    \fn audioCodecSelect
    \brief Update UI
*/
uint8_t DIA_audioCodec( int *codec );
void audioCodecSelect( void )
{
//#warning FIXME
//#warning FIXME
//#warning FIXME
	//DIA_audioCodec( &currentEncoder );
	audioPrintCurrentCodec(0);
}
/**
    \fn     audioCodecSetByName
    \brief  only called by JS, we have to update UI as well
*/
uint8_t audioCodecSetByName( int dex,const char *name)
{
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    ADM_info("Setting %s audio codec for track %d\n",name,dex);
    if(!ed)
    {
        ADM_warning("Cannot set codec for track %d\n",dex);
        return 0;
    }


		for(uint32_t i=0;i<ListOfAudioEncoder.size();i++)
		{
			if(!strcasecmp(name,ListOfAudioEncoder[i]->codecName))
			{

				ed->encoderIndex=i;
                                if(!dex)
                                        audioPrintCurrentCodec(dex); // Update UI
                                ADM_info("Audio codec %s set for track %d (index=%d)\n",name,dex,i);
				return 1;
			}

		}
		ADM_warning("\n Mmmm Select audio codec by name failed...(%s) for track %d\n",name,dex);
		return 0;
}
/**
    \fn audioCodecSetByIndex
    \brief To be used by UI code only!
*/
uint8_t audioCodecSetByIndex(int dex,int i)
{
    ADM_assert(i<ListOfAudioEncoder.size());
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot set codec for track %d\n",dex);
        return 0;
    }
    ed->encoderIndex=i;
    if(ed->encoderConf) delete ed->encoderConf;
    ed->encoderConf=NULL;
    printf("[AudioEncoder] Selected %s for index %d, tag 0x%x \n",
        ListOfAudioEncoder[i]->codecName,i,ListOfAudioEncoder[i]->wavTag);
    return 1;

}
/**
    \fn audioCodecGetName
    \brief Returns the current codec tagname
*/
const char *audioCodecGetName( int dex )
{

    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot get codec name for track %d\n",dex);
        return 0;
    }
    ADM_assert(ed->encoderIndex<ListOfAudioEncoder.size());
    return ListOfAudioEncoder[ed->encoderIndex]->codecName;

}

/**
    \fn audioProcessMode
    \brief
    @return 1 in process mode, 0 in copy mode
*/
uint32_t audioProcessMode(int dex)
{

    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot get audio codec mode for track %d\n",dex);
        return 0;
    }
    ADM_assert(ed->encoderIndex<ListOfAudioEncoder.size());

    if(!ed->encoderIndex) return 0;
    return 1;
}
/**
 * 	\fn getAudioOuputTag
 *  \brief Return the encoding of the currently selected codec
 *  Must be called only in process mode, else it is meaningless.
 */
uint32_t audioFilter_getOuputCodec(int dex)
{
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot get audio output codec for track %d\n",dex);
        return 0;
    }
    return ListOfAudioEncoder[ed->encoderIndex]->wavTag;
}

/**
 * 	\fn audioFilter_getMaxChannels
 *  \brief Return the max # of channels a codec supports
 */
uint32_t audioFilter_getMaxChannels(int dex)
{
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot get audio maxc channel for track %d\n",dex);
        return 9999;
    }
    ADM_assert(ed->encoderIndex<ListOfAudioEncoder.size());
	if(!ed->encoderIndex) return 99999;
	return ListOfAudioEncoder[ed->encoderIndex]->maxChannels;
}
/**
    \fn audioCodecConfigure
    \brief
*/
void audioCodecConfigure( int dex )
{
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot get audio configure for track %d\n",dex);
        return;
    }
    int i=ed->encoderIndex;
    ADM_info("Track %d has %d audio encoder index\n",dex,i);
    if(ListOfAudioEncoder[i]->configure)
    {
        ListOfAudioEncoder[i]->configure(&(ed->encoderConf));
    }
    return;
}
/**
    \fn audioCodecConfigureCodecIndex
    \brief
*/
void audioCodecConfigureCodecIndex( int dex,CONFcouple **conf  )
{
    if(dex>=ListOfAudioEncoder.size())
    {
            ADM_warning("Audio codec out of bound %d/%d\n",dex,ListOfAudioEncoder.size());
            return;
    }
    if(ListOfAudioEncoder[dex]->configure)
    {
        ListOfAudioEncoder[dex]->configure(conf);
    }
    return;
}

/**
        \fn audioEncoderCreate
        \brief Spawn an audio encoder
*/
ADM_AudioEncoder *audioEncoderCreate(EditableAudioTrack *ed,AUDMAudioFilter *filter,bool globalHeader)
{
    if(!ed)
    {
        ADM_warning("audioEncoderCreate : NULL track %d\n");
        return NULL;
    }
      ADM_assert(ed->encoderIndex<ListOfAudioEncoder.size());
      ADM_audioEncoder *enc=ListOfAudioEncoder[ed->encoderIndex];
      ADM_AudioEncoder *a= enc->create(filter,globalHeader,(ed->encoderConf));
      return a;
}
/**
        \fn audioEncoderCreate
        \brief Spawn an audio encoder
*/
ADM_AudioEncoder *audioEncoderCreate(int dex,AUDMAudioFilter *filter,bool globalHeader)
{
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot create encoder for track %d\n",dex);
        return NULL;
    }
    return audioEncoderCreate(ed,filter,globalHeader);
}
/**
        \fn getAudioExtraConf
        \brief
*/

bool getAudioExtraConf(int dex,CONFcouple **couple)
{
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot get audio bitrate for track %d\n",dex);
        return false;
    }
    int i=ed->encoderIndex;
    if(!i)
    {
        printf("[AudioEncoder] Cannot get conf on copy!\n");
        return false;
    }
     ADM_assert(i<ListOfAudioEncoder.size());
     *couple=CONFcouple::duplicate(ed->encoderConf); // duplicate ?
      return true;
}
/**
    \fn setAudioExtraConf
*/
bool setAudioExtraConf(int dex,CONFcouple *c)
{

   EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot get audio bitrate for track %d\n",dex);
        return false;
    }
    if(!ed->encoderIndex)
    {
        printf("[AudioEncoder] Cannot set conf on copy! (track %d)\n",dex);
        return false;
    }
    ADM_assert(ed->encoderIndex<ListOfAudioEncoder.size());
    if(ed->encoderConf) delete ed->encoderConf;
    ed->encoderConf=NULL;
    if(c)
        ed->encoderConf=CONFcouple::duplicate(c);
    return true;
}
/**
        \fn audio_selectCodecByTag
        \brief Select the "best" encoder outputing tag codec
*/
uint8_t audio_selectCodecByTag(int dex,uint32_t tag)
{
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot set audio from tag for track %d\n",dex);
        return 0;
    }
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
        if(ed->encoderIndex!=selected)
        {
            ed->encoderIndex=selected;
            if(ed->encoderConf) delete ed->encoderConf;
            ed->encoderConf=NULL;
        }
        if(!dex)
            UI_setAudioCodec( (int)ed->encoderIndex);
        printf("[AudioEncoder] Selected %s for tag %d (%s)\n",ListOfAudioEncoder[ed->encoderIndex]->codecName,tag,"");
        return 1;
    }
    return 0;
}
/**
         \fn audioSetOption
         \brief Allow per codec switch
*/
uint8_t audioSetOption(int dex,const char *option, uint32_t value)
{
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot set audio from tag for track %d\n",dex);
        return 0;
    }
    ADM_audioEncoder *c=ListOfAudioEncoder[ed->encoderIndex];
    if(!c->setOption) return 0;
    return c->setOption(&ed->encoderConf,option,value);
}
/**
         \fn audio_setCopyCodec
         \brief Set audio codec to copy
*/
uint8_t audio_setCopyCodec(int dex)
{
    EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(dex);
    if(!ed)
    {
        ADM_warning("Cannot set audio from tag for track %d\n",dex);
        return 0;
    }
    ed->encoderIndex=0;
    return 1;

}

//**
