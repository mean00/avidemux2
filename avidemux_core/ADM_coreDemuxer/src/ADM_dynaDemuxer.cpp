/***************************************************************************
                          ADM_coreDemuxer.cpp  -  description
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
#include "ADM_demuxerInternal.h"
bool ADM_dm_cleanup(void);
void ADM_demuxersCleanup(void);
vidHeader *ADM_demuxerSpawn(uint32_t magic,const char *name);

BVector <ADM_demuxer *> ListOfDemuxers;


/**
        \fn ADM_dm_getNbDemuxers
        \brief Returns the number of demuxers plugins except one
*/
uint32_t ADM_dm_getNbDemuxers(void)
{
    return ListOfDemuxers.size();
}
/**
    \fn     ADM_dm_getDemuxerInfo
    \brief  Get Infos about the demuxer #th plugin
*/
bool     ADM_dm_getDemuxerInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch)
{
    ADM_assert(filter<ListOfDemuxers.size());
    ListOfDemuxers[filter]->getVersion(major,minor,patch);
    *name=ListOfDemuxers[filter]->descriptor;
    return true;
}
/**
    \fn tryLoadingFilterPlugin
    \brief Try loading the file given as argument as an audio device plugin

*/
#define Fail(x) {printf("%s:"#x"\n",file);goto er;}
static bool tryLoadingDemuxerPlugin(const char *file)
{
	ADM_demuxer *dll=new ADM_demuxer(file);
    if(!dll->initialised) Fail(CannotLoad);
    if(dll->apiVersion!=ADM_DEMUXER_API_VERSION) Fail(WrongApiVersion);

    ListOfDemuxers.append(dll); // Needed for cleanup. FIXME TODO Delete it.
    printf("[Demuxers] Registered filter %s as  %s\n",file,dll->descriptor);
    return true;
	// Fail!
er:
	delete dll;
	return false;

}
/**
 * 	\fn ADM_dm_loadPlugins
 *  \brief load all audio device plugins
 */
uint8_t ADM_dm_loadPlugins(const char *path)
{
#define MAX_EXTERNAL_FILTER 100
// FIXME Factorize

	char *files[MAX_EXTERNAL_FILTER];
	uint32_t nbFile;

	memset(files,0,sizeof(char *)*MAX_EXTERNAL_FILTER);
	printf("[ADM_dm_plugin] Scanning directory %s\n",path);

	if(!buildDirectoryContent(&nbFile, path, files, MAX_EXTERNAL_FILTER, SHARED_LIB_EXT))
	{
		printf("[ADM_av_plugin] Cannot parse plugin\n");
		return 0;
	}

	for(int i=0;i<nbFile;i++)
		tryLoadingDemuxerPlugin(files[i]);
    int nb=ListOfDemuxers.size();
	
    // Now sort them according to priority
    
    for(int i=0;i<nb;i++)
        for(int j=i+1;j<nb;j++) // sub optimal, but who cares
        {
            if(ListOfDemuxers[i]->priority<ListOfDemuxers[j]->priority)
                {
                        ADM_demuxer *a=ListOfDemuxers[i];
                        ListOfDemuxers[i]=ListOfDemuxers[j];
                        ListOfDemuxers[j]=a;
                }
        }
    for(int i=0;i<nb;i++)
        ADM_info("Demuxer plugin : %s, priority :%d\n",ListOfDemuxers[i]->name,(int)ListOfDemuxers[i]->priority);
    printf("[ADM_dm_plugin] Scanning done, %d demuxers found\n",(int)nb);
        clearDirectoryContent(nbFile,files);
	return 1;
}
/**
    \fn ADM_dm_destroy
*/
bool ADM_dm_cleanup(void)
{
    ADM_demuxersCleanup();
    return true;
}
/**
        \fn AVDM_cleanup
        \brief Current device is no longer used, delete
*/
void ADM_demuxersCleanup(void)
{
        int nb=ListOfDemuxers.size();
        for(int i=0;i<nb;i++)
                {
                        if(ListOfDemuxers[i]) delete ListOfDemuxers[i];
                        ListOfDemuxers[i]=NULL;
                }
}
/**
    \fn ADM_demuxerSpawn
    \brief Locate the correct demuxer and instantiate it

*/
vidHeader *ADM_demuxerSpawn(uint32_t magic,const char *name)
{
int found=-1;
uint32_t score=0;
uint32_t mark;
    for(int i=0;i<ListOfDemuxers.size();i++)
    {
        mark=ListOfDemuxers[i]->probe(magic,name);
        if(mark>score)
        {
            score=mark;
            found=i;
        }
    }
    if(score && found!=-1)
    {
        return ListOfDemuxers[found]->createdemuxer();
    }
    return NULL;
}

// EOF

