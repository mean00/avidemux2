/***************************************************************************
                          ADM_coreMuxer.cpp  -  description
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
#include <vector>
#include "ADM_default.h"
#include "ADM_muxerInternal.h"

void ADM_muxersCleanup(void);
ADM_muxer *ADM_muxerSpawn(uint32_t magic,const char *name);

std::vector <ADM_dynMuxer *> ListOfMuxers;
/**
    \fn ADM_mux_configure
    \brief 
*/
bool ADM_mux_configure(int index)
{
    ADM_assert(index<ListOfMuxers.size());
    return ListOfMuxers[index]->configure();
}

/**
        \fn ADM_mx_getNbMuxers
        \brief Returns the number of muxers plugins except one
*/
uint32_t ADM_mx_getNbMuxers(void)
{
    return ListOfMuxers.size();
}
/**
    \fn ADM_mx_getDisplayName
    \brief Returns display name for muxer i
*/
const char *ADM_mx_getDisplayName(uint32_t i)
{
    ADM_assert(i<ListOfMuxers.size());
    return ListOfMuxers[i]->displayName;
}
/**
    \fn ADM_mx_getName
    \brief Returns internal name for muxer i
*/
const char *ADM_mx_getName(uint32_t i)
{
    ADM_assert(i<ListOfMuxers.size());
    return ListOfMuxers[i]->name;
}

/**
    \fn     ADM_dm_getDemuxerInfo
    \brief  Get Infos about the demuxer #th plugin
*/
bool     ADM_mx_getMuxerInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch)
{
    ADM_assert(filter<ListOfMuxers.size());
    ListOfMuxers[filter]->getVersion(major,minor,patch);
    *name=ListOfMuxers[filter]->descriptor;
    return true;
}
/**
    \fn tryLoadingMuxerPlugin
    \brief Try loading the file given as argument as a muxer

*/
#define Fail(x) {printf("%s:"#x"\n",file);goto er;}
static bool tryLoadingMuxerPlugin(const char *file)
{
	ADM_dynMuxer *dll=new ADM_dynMuxer(file);
    if(!dll->initialised) Fail(CannotLoad);
    if(dll->apiVersion!=ADM_MUXER_API_VERSION) Fail(WrongApiVersion);

    ListOfMuxers.push_back(dll); // Needed for cleanup. FIXME TODO Delete it.
    printf("[Muxers] Registered filter %s as  %s\n",file,dll->descriptor);
    return true;
	// Fail!
er:
	delete dll;
	return false;

}
/**
 * 	\fn ADM_mx_loadPlugins
 *  \brief load all audio device plugins
 */
uint8_t ADM_mx_loadPlugins(const char *path)
{
#define MAX_EXTERNAL_FILTER 100
// FIXME Factorize

	char *files[MAX_EXTERNAL_FILTER];
	uint32_t nbFile;

	memset(files,0,sizeof(char *)*MAX_EXTERNAL_FILTER);
	printf("[ADM_mx_plugin] Scanning directory %s\n",path);

	if(!buildDirectoryContent(&nbFile, path, files, MAX_EXTERNAL_FILTER, SHARED_LIB_EXT))
	{
		printf("[ADM_av_plugin] Cannot parse plugin\n");
		return 0;
	}

	for(int i=0;i<nbFile;i++)
		tryLoadingMuxerPlugin(files[i]);

	printf("[ADM_mx_plugin] Scanning done\n");

	return 1;
}
/**
        \fn ADM_MuxersCleanup
        \brief Current device is no longer used, delete
*/
void ADM_MuxersCleanup(void)
{
        int nb=ListOfMuxers.size();
        for(int i=0;i<nb;i++)
                {
                        if(ListOfMuxers[i]) delete ListOfMuxers[i];
                        ListOfMuxers[i]=NULL;
                }
}
/**
    \fn ADM_MuxerSpawn
    \brief Locate the correct demuxer and instantiate it

*/
ADM_muxer *ADM_MuxerSpawn(const char *name)
{
int found=-1;
uint32_t score=0;
uint32_t mark;
    for(int i=0;i<ListOfMuxers.size();i++)
    {
       
            score=1;
            found=i;
        
    }
    if(score && found!=-1)
    {
        return ListOfMuxers[found]->createmuxer();
    }
    return NULL;
}

/**
    \fn ADM_MuxerSpawnFromIndex
    \brief Locate the correct demuxer and instantiate it

*/
ADM_muxer *ADM_MuxerSpawnFromIndex(int index)
{
    ADM_assert(index<ListOfMuxers.size());
    return ListOfMuxers[index]->createmuxer();
}
//___________________________________________
extern "C"
{
    #include "ADM_libraries/ADM_ffmpeg/libavformat/avformat.h"
    #include "ADM_libraries/ADM_ffmpeg/libavformat/avio.h" // to get regiter protocol
};
extern "C"
{
extern void movenc_init(void);
extern void flvenc_init(void);
extern void matroskaenc_init(void);
}
extern struct URLProtocol file_protocol ;
/**


*/
void ADM_lavFormatInit(void)
{
    // Init lavformat

        movenc_init();
        flvenc_init();
        matroskaenc_init();
        register_protocol(&file_protocol);
}
// EOF

