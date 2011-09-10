/***************************************************************************
    \file       ADM_dynVideoDecoder.cpp
    \brief      Handle video decoder plugins
    \author     mean/gruntster fixounet@free.fr (C) 2010
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
#include <BVector.h>

#include "DIA_fileSel.h"
#include "ADM_coreVideoDecoderInternal.h"
#include "ADM_dynamicLoading.h"

/**
    \class ADM_videoEncoder6
    \brief Plugin Wrapper Class

*/
class ADM_videoDecoder6 :public ADM_LibWrapper
{
public:
        int                  initialised;
        ADM_videoDecoderDesc *desc;
        ADM_videoDecoderDesc  *(*getInfo)();
        ADM_videoDecoder6(const char *file) : ADM_LibWrapper()
        {
                initialised = (loadLibrary(file) && getSymbols(1,
				&getInfo, "getInfo"));
                if(initialised)
                {
                    desc=getInfo();
                    printf("[videoDecoder6]Name :%s ApiVersion :%d Description :%s\n",
                                                        desc->decoderName,
                                                        desc->apiVersion,
                                                        desc->description);
                }else
                {
                    printf("[videoDecoder6]Symbol loading failed for %s\n",file);
                }
        }
};

BVector <ADM_videoDecoder6 *> ListOfDecoders;
// 

/**
        \fn ADM_vd6_getNbEncoders
        \brief Returns the number of demuxers plugins except one
*/
uint32_t ADM_vd6_getNbEncoders(void)
{
    return ListOfDecoders.size();
}
/**
    \fn     ADM_ve6_getEncoderInfo
    \brief  Get Infos about the demuxer #th plugin
*/
bool     ADM_vd6_getEncoderInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch)
{
    ADM_assert(filter<ListOfDecoders.size());
    ADM_videoDecoderDesc *desc=ListOfDecoders[filter]->desc;
    ADM_assert(desc);
    *name=desc->menuName;
    *major=desc->major;
    *minor=desc->minor;
    *patch=desc->patch;
    return true;
}
/**
    \fn tryLoadingFilterPlugin
    \brief Try loading the file given as argument as an audio device plugin

*/
#define Fail(x) {printf("%s:"#x"\n",file);goto er;}
static bool tryLoadingEncoderPlugin(const char *file)
{
	ADM_videoDecoder6 *dll=new ADM_videoDecoder6(file);
    if(!dll->initialised) Fail(CannotLoad);
    if(dll->desc->apiVersion!=ADM_VIDEO_DECODER_API_VERSION) Fail(WrongApiVersion);
//fixme todo also check uiType    
    ListOfDecoders.append(dll); // Needed for cleanup. FIXME TODO Delete it.
    printf("[VideoDecoder6] Registered filter %s as  %s\n",file,dll->desc->description);
    return true;
	// Fail!
er:
	delete dll;
	return false;

}
/**
 * 	\fn ADM_vd6_loadPlugins
 *  \brief load all video decoder plugins
 */
uint8_t ADM_vd6_loadPlugins(const char *path)
{
#define MAX_EXTERNAL_FILTER 100
// FIXME Factorize

	char *files[MAX_EXTERNAL_FILTER];
	uint32_t nbFile;

	memset(files,0,sizeof(char *)*MAX_EXTERNAL_FILTER);
	printf("[ADM_vd6_plugin] Scanning directory %s\n",path);

	if(!buildDirectoryContent(&nbFile, path, files, MAX_EXTERNAL_FILTER, SHARED_LIB_EXT))
	{
		printf("[ADM_vd6_plugin] Cannot parse plugin\n");
		return 0;
	}

	for(int i=0;i<nbFile;i++)
		tryLoadingEncoderPlugin(files[i]);
    
	printf("[ADM_vd6_plugin] Scanning done\n");
        clearDirectoryContent(nbFile,files);

	return 1;
}
/**
        \fn ADM_ve6_cleanup
        \brief Current device is no longer used, delete
*/
void ADM_vd6_cleanup(void)
{
        int nb=ListOfDecoders.size();
        for(int i=0;i<nb;i++)
                {
                        if(ListOfDecoders[i]) delete ListOfDecoders[i];
                        ListOfDecoders[i]=NULL;
                }
}

/**
    \fn ADM_vd6_getMenuName
    \brief 
*/
const char *ADM_vd6_getMenuName(uint32_t i)
{
	 int nb=ListOfDecoders.size();

	ADM_assert(i < nb);

	return ListOfDecoders[i]->desc->menuName;
}
/**
    \fn createVideoEncoder
*/
static decoders *createVideoDecoderFromIndex(int index,uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen,
                    uint8_t *extra, uint32_t bpp)
{
    int nb=ListOfDecoders.size();
	ADM_assert(index < nb);
    ADM_videoDecoder6 *plugin=ListOfDecoders[index];

    decoders *dec=plugin->desc->create(w,h,fcc,extraDataLen,extra,bpp);
    return dec;
}
/**
    \fn createVideoEncoderFromIndex
    \brief Spawn a decoder. If not possible returns NULL.
*/
decoders *tryCreatingVideoDecoder(uint32_t w, uint32_t h, uint32_t fcc,uint32_t extraDataLen,
                    uint8_t *extra, uint32_t bpp)
{

     int nb=ListOfDecoders.size();
     for(int i=0;i<nb;i++)
     {
            ADM_videoDecoder6 *plugin=ListOfDecoders[i];
            uint32_t *f=plugin->desc->fccs;
            if(!f) continue;
            while(*f)
            {
                if(fcc==*f)
                {
                    decoders *dec=createVideoDecoderFromIndex(i,w,h,fcc,extraDataLen,extra,bpp);
                    if(dec) return dec;
                }
                f++;
            }
     }
     ADM_info("No decoder found in plugin\n");
     return NULL;
}
//EOF
