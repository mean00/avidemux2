/***************************************************************************
                             ADM_dynVideoEncoder.cpp

    begin                : Mon Apr 14 2008
    copyright            : (C) 2008 2009 by gruntster/mean
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
#include <vector>

#include "DIA_fileSel.h"

#include "ADM_coreVideoEncoder.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "DIA_uiTypes.h"
#include "ADM_dynamicLoading.h"
/**
    \class ADM_videoEncoder6
    \brief Plugin Wrapper Class

*/
class ADM_videoEncoder6 :public ADM_LibWrapper
{
public:
        int                  initialised;
        ADM_videoEncoderDesc *desc;
        ADM_videoEncoderDesc  *(*getInfo)();
        ADM_videoEncoder6(const char *file) : ADM_LibWrapper()
        {
			initialised = (loadLibrary(file) && getSymbols(1,
				&getInfo, "getInfo"));
                if(initialised)
                {
                    desc=getInfo();
                    printf("[videoEncoder6]Name :%s ApiVersion :%d Description :%s\n",
                                                        desc->encoderName,
                                                        desc->apiVersion,
                                                        desc->description);
                }else
                {
                    printf("[videoEncoder6]Symbol loading failed for %s\n",file);
                }
        }
};

std::vector <ADM_videoEncoder6 *> ListOfEncoders;


/**
        \fn ADM_ve6_getNbDemuxers
        \brief Returns the number of demuxers plugins except one
*/
uint32_t ADM_ve6_getNbDemuxers(void)
{
    return ListOfEncoders.size();
}
/**
    \fn     ADM_dm_getDemuxerInfo
    \brief  Get Infos about the demuxer #th plugin
*/
bool     ADM_ve6_getDemuxerInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch)
{
    ADM_assert(filter<ListOfEncoders.size());
    ADM_videoEncoderDesc *desc=ListOfEncoders[filter]->desc;
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
	ADM_videoEncoder6 *dll=new ADM_videoEncoder6(file);
    if(!dll->initialised) Fail(CannotLoad);
    if(dll->desc->apiVersion!=ADM_VIDEO_ENCODER_API_VERSION) Fail(WrongApiVersion);
//fixme todo also check uiType    
    ListOfEncoders.push_back(dll); // Needed for cleanup. FIXME TODO Delete it.
    printf("[VideoEncoder6] Registered filter %s as  %s\n",file,dll->desc->description);
    return true;
	// Fail!
er:
	delete dll;
	return false;

}
/**
 * 	\fn ADM_ve6_loadPlugins
 *  \brief load all audio device plugins
 */
uint8_t ADM_ve6_loadPlugins(const char *path)
{
#define MAX_EXTERNAL_FILTER 100
// FIXME Factorize

	char *files[MAX_EXTERNAL_FILTER];
	uint32_t nbFile;

	memset(files,0,sizeof(char *)*MAX_EXTERNAL_FILTER);
	printf("[ADM_ve6_plugin] Scanning directory %s\n",path);

	if(!buildDirectoryContent(&nbFile, path, files, MAX_EXTERNAL_FILTER, SHARED_LIB_EXT))
	{
		printf("[ADM_ve6_plugin] Cannot parse plugin\n");
		return 0;
	}

	for(int i=0;i<nbFile;i++)
		tryLoadingEncoderPlugin(files[i]);

	printf("[ADM_ve6_plugin] Scanning done\n");

	return 1;
}
/**
        \fn ADM_ve6_cleanup
        \brief Current device is no longer used, delete
*/
void ADM_ve6_cleanup(void)
{
        int nb=ListOfEncoders.size();
        for(int i=0;i<nb;i++)
                {
                        if(ListOfEncoders[i]) delete ListOfEncoders[i];
                        ListOfEncoders[i]=NULL;
                }
}

//EOF
