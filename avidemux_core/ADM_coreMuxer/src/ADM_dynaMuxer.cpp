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

#include "BVector.h"
#include "ADM_default.h"
#include "ADM_muxerInternal.h"
#include "ADM_muxerProto.h"

extern "C" {
#include "libavformat/url.h"
}

void ADM_MuxersCleanup(void);

ADM_muxer *ADM_muxerSpawn(uint32_t magic,const char *name);

BVector <ADM_dynMuxer *> ListOfMuxers;
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
    \fn ADM_MuxerGetDefaultExtension
    \brief returns the default extension (i.e. mkv, avi) from index
*/
const char *ADM_MuxerGetDefaultExtension(int i)
{
    ADM_assert(i<ListOfMuxers.size());
    return ListOfMuxers[i]->defaultExtension;
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

    ListOfMuxers.append(dll); // Needed for cleanup. FIXME TODO Delete it.
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
    // Sort muxers by displayName, bubble sort
    int nb=ListOfMuxers.size();
    for(int i=0;i<nb;i++)
        for(int j=i+1;j<nb;j++)
        {
             ADM_dynMuxer *a,*b;
             a=ListOfMuxers[i];
             b=ListOfMuxers[j];
             if(strcmp(a->displayName,b->displayName)>0)
             {
                ListOfMuxers[j]=a;
                ListOfMuxers[i]=b;
             }
        }
        clearDirectoryContent(nbFile,files);
	return 1;
}
/**
    \fn ADM_mx_cleanup
*/
bool ADM_mx_cleanup(void) 
{
    ADM_MuxersCleanup();
    return true;
}
/**
    \fn ADM_mx_getExtraConf
    \brief Retrieve extra configuration from current muxer
*/
bool ADM_mx_getExtraConf(int index,CONFcouple **c)
{
    *c=NULL;
    if(index>=ListOfMuxers.size())
    {
        ADM_error("Given index exceeds muxer list\n",index);
        return false;
    }
    ADM_dynMuxer *mux=ListOfMuxers[index];
    return mux->getConfiguration(c);
    
}
/**
    \fn ADM_mx_setExtraConf
    \brief Set extra configuration from current muxer
*/
bool ADM_mx_setExtraConf(int index,CONFcouple *c)
{
    if(!c) return true;
    if(index>=ListOfMuxers.size())
    {
        ADM_error("Given index exceeds muxer list\n",index);
        return false;
    }
    ADM_dynMuxer *mux=ListOfMuxers[index];
    return mux->setConfiguration(c);
    
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
    \fn ADM_muxerIndexFromName
    \brief return muxer index from name, -1 if error
*/
int ADM_MuxerIndexFromName(const char *name)
{
    int n=ListOfMuxers.size();
    for(int i=0;i<n;i++)
    {
        ADM_dynMuxer *mux=ListOfMuxers[i];
        if(!strcasecmp(mux->name,name)) return i;
    }
    return -1;
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
    found=ADM_MuxerIndexFromName(name);
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
    #include "libavformat/avformat.h"
};

void ADM_lavFormatInit(void)
{
	av_register_all();

	// Make sure avformat is correctly configured
	const char* formats[] = {"mpegts", "dvd", "vcd", "svcd", "mp4", "psp", "flv", "matroska"};
	AVOutputFormat *avfmt;

	for (int i = 0; i < 8; i++)
	{
		avfmt = av_guess_format(formats[i], NULL, NULL);

		if (avfmt == NULL)
		{
			printf("Error: %s muxer isn't registered\n", formats[i]);
			ADM_assert(0);
		}
	}

         const URLProtocol **prot=ffurl_get_protocols("file",NULL);
         bool found=false;
         if(prot)
         {
                if (!strcmp(prot[0]->name, "file"))
                {
                        found=true;
                }
         } 
         if(!found)
         {
		printf("Error: file protocol isn't registered\n");
		ADM_assert(0);
	}
}
// EOF

