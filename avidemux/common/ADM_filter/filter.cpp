/***************************************************************************
                          filter.cpp  -  description
                             -------------------
    begin                : Wed Mar 27 2002
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
#if 0
#include "ADM_default.h"
#include <vector>
#include "fourcc.h"
#include "DIA_fileSel.h"
#include "ADM_dynamicLoading.h"

#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_video/ADM_videoNull.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_videoFilterDynamic.h"
#include "ADM_video/ADM_vidPartial.h"
#include "ADM_render/GUI_render.h"

#include "ADM_debugID.h"
#define MODULE_NAME MODULE_PREVIEW
#include "ADM_debug.h"
#include "ADM_quota.h"
#include "ADM_preview.h"
//

extern ADM_UI_TYPE UI_GetCurrentUI(void);

class ADM_vf_pluginLoader : public ADM_LibWrapper
{
	public:
		VF_getDescriptor		*getDesc;
		ADM_vf_pluginLoader(const char *file) : ADM_LibWrapper()
		{
				getDesc=NULL;
				if(loadLibrary(file))
					getDesc=(VF_getDescriptor *)getSymbol("ADM_VF_getDescriptor");
		};
};
// exported vars
std::vector <FilterDescriptor *> allfilters;
std::vector <FilterDescriptor *> filterCategories[VF_MAX];
std::vector <ADM_vf_pluginLoader *> pluginLoaderQueue;

uint32_t nb_active_filter=0;
FILTER  videofilters[VF_MAX_FILTER];



static uint32_t lastStart=0, lastNb=0;
static uint32_t tagCount=VF_START_TAG;

extern ADM_Composer *video_body;
extern AVDMGenericVideoStream *filterCreateFromTag(VF_FILTERS tag,CONFcouple *conf, AVDMGenericVideoStream *in) ;
extern char *ADM_escape(const ADM_filename *incoming);
//

static  void updateVideoFilters(void);

// Ugly should be dynamically allocated
#warning HARDCODEC IMAGE SIZE

// dummy constructor used to register the filter
//____________________________________
AVDMVideo_FilterDec::AVDMVideo_FilterDec(char *name,
									AVDMGenericVideoStream *(*create) (AVDMGenericVideoStream *in, void *))
{
    UNUSED_ARG(name);
    UNUSED_ARG(create);
         //registerFilter(name, create);
	 //
}
//
// Delete everything that is pending
void filterCleanUp( void )
{
	for(uint32_t i=0;i<nb_active_filter;i++)
	{
		  	delete videofilters[i].filter;
			if(videofilters[i].conf) delete videofilters[i].conf;
             videofilters[i].conf=NULL;
             videofilters[i].filter=NULL;
	}
   nb_active_filter=0;
}
void filterListAll( void )
{
const char *name;
	printf("\nVideo filters\n");
	for(uint32_t i=0;i<allfilters.size();i++)
	{
		name=allfilters[i]->filterName;

		{
			if(name)
				printf("\t%s\n",name);
		}
	}

}
/**
 *      \fn filterGetTagFromName
 *      \Brief returns the tag associated with a name, invalid otherwise
 */
VF_FILTERS filterGetTagFromName(const char *inname)
{
const char *name;
int max=allfilters.size();
	VF_FILTERS filter=VF_INVALID;
	for(uint32_t i=0;i<max;i++)
	{
		name=allfilters[ i]->filterName;
		if(1) // allfilters[i]->tag==VF_PARTIAL_FILTER)
		{
			if(name)
			{
				if(strlen(name))
				{
					if(!strcasecmp(name,inname))
						return allfilters[ i]->tag;
				}
			}
		}
	}
	return filter;
}
/**
 * 	\fn tryLoadingFilterPlugin
 *  \brief try to load the plugin given as argument..
 */

#define Fail(x) {printf("%s:"#x"\n",file);goto er;}
static bool tryLoadingFilterPlugin(const char *file)
{
	ADM_vf_pluginLoader *dll=new ADM_vf_pluginLoader(file);

	FilterDescriptor *desc=NULL;
	FilterDescriptor *myDesc=NULL;
	if(!dll->getDesc) Fail(nogetdesc);

	desc=dll->getDesc();
	if(!desc) Fail(GetDescriptor);
	// Check the API version
	if(desc->apiVersion!=ADM_FILTER_API_VERSION) Fail(WrongAPI);
	if(!(desc->uiFlags & UI_GetCurrentUI())) Fail(WrongUI);
	// Duplicate it, just in case...
	myDesc=new FilterDescriptor();
	memcpy(myDesc,desc,sizeof(*myDesc));
	// push it !
	myDesc->tag=tagCount++;
	ADM_assert(myDesc->category<VF_MAX);

    allfilters.push_back (myDesc);
    filterCategories[myDesc->category].push_back(myDesc);
    pluginLoaderQueue.push_back(dll); // Needed for cleanup. FIXME TODO Delete it.
    printf("[Filters] Registered filter %s as  %s\n",file,desc->name);
    return true;
	// Tag it
	// Fail!
er:
	delete dll;
	return false;

}
/**
 * 	\fn ADM_vf_loadPlugins
 *  \brief load all video plugins
 */
uint8_t ADM_vf_loadPlugins(const char *path)
{
#define MAX_EXTERNAL_FILTER 100


	char *files[MAX_EXTERNAL_FILTER];
	uint32_t nbFile;

	memset(files,0,sizeof(char *)*MAX_EXTERNAL_FILTER);
	printf("[ADM_vf_plugin] Scanning directory %s\n",path);

	if(!buildDirectoryContent(&nbFile, path, files, MAX_EXTERNAL_FILTER, SHARED_LIB_EXT))
	{
		printf("[ADM_vf_plugin] Cannot parse plugin\n");
		return 0;
	}

	for(int i=0;i<nbFile;i++)
		tryLoadingFilterPlugin(files[i]);

	printf("[ADM_vf_plugin] Scanning done\n");

	return 1;
}
/**
 * 	\fn registerFilterEx
 *  \brief Register a video filter
 */
void registerFilterEx(const char *name,const char *filtername,VF_CATEGORY category,
		AVDMGenericVideoStream *(*create) (AVDMGenericVideoStream *in, CONFcouple *),
		AVDMGenericVideoStream *(*create_from_script) (AVDMGenericVideoStream *in, int n,Arg *args),
		const char *descText)
{

	FilterDescriptor *desc=new FilterDescriptor(tagCount++,filtername,name,
												descText,
												category,
												create,
												create_from_script);

		ADM_assert(desc->category<VF_MAX);

        allfilters.push_back (desc);
        filterCategories[desc->category].push_back(desc);

        aprintf("[Filters] Registered filter %s\n",desc->name);


}
/************************** Get the last video filter of the chain ***************************
The reference is 0 in that case !
************************************************************************************************/
AVDMGenericVideoStream *getLastVideoFilter(void)
{
    aviInfo info;

	video_body->getVideoInfo(&info);

	lastNb=info.nb_frames;
	lastStart=0;
	aprintf("GetLast : -- LasrNB %lu start %lu \n",lastNb,lastNb);
  // sanity check
  	if(nb_active_filter==0)
  		{
  		 		nb_active_filter=1;
				aprintf("--preview filter 0\n");
				return videofilters[  nb_active_filter-1].filter;
  		}
 	updateVideoFilters();
	return videofilters[  nb_active_filter-1].filter;
}

/*
	Return last video filter and rebuild a chain with only the selected frame

*/
AVDMGenericVideoStream *getLastVideoFilter( uint32_t start, uint32_t nb)
{
  // sanity check
  	lastNb=nb;
	lastStart=start;
	aprintf("GetLast full : -- LasrNB %lu start %lu \n",lastNb,lastNb);
  	if(nb_active_filter==0)
  		{
  		 		nb_active_filter=1;
  		 		videofilters[0].filter=  new AVDMVideoStreamNull(video_body,start,nb);
  		}
	updateVideoFilters();
       return videofilters[  nb_active_filter-1].filter;
}

/********* Returns the first video filters starting from frame xx with nb frames ***************/
AVDMGenericVideoStream *getFirstVideoFilter( uint32_t start, uint32_t nbFrame)
{
  // sanity check
  	lastNb=nbFrame;
	lastStart=start;
	aprintf("GetFirst full : -- LasrNB %lu start %lu \n",lastNb,lastNb);
	updateVideoFilters();
       return videofilters[  0].filter;
}
AVDMGenericVideoStream *getFirstCurrentVideoFilter( void)
{
	ADM_assert(nb_active_filter);
 	return videofilters[  0].filter;
}
AVDMGenericVideoStream *getFirstVideoFilter( void)
{
  // sanity check
  aviInfo info;

	video_body->getVideoInfo(&info);
	lastNb=info.nb_frames;
	lastStart=0;
	aprintf("GetFirst : -- LasrNB %lu start %lu \n",lastNb,lastNb);
	updateVideoFilters();
       return videofilters[  0].filter;

}

FILTER * getCurrentVideoFilterList (uint32_t * count)
{
    if (count)
        *count = nb_active_filter;
    return videofilters;
}

//
//	Parse the list of active filters, delete them et recreate them with (new ?) configuration
//
void updateVideoFilters(void )
{
		if(nb_active_filter!=0)
		{
			delete videofilters[0].filter;
			videofilters[0].filter=NULL;

		}
  		 videofilters[0].filter=  new AVDMVideoStreamNull(video_body,lastStart,lastNb);

  		if(nb_active_filter<=1)
  		{
  		 	nb_active_filter=1;
			aprintf("--preview filter %d\n",nb_active_filter-1);
                        admPreview::updateFilters(videofilters[0].filter,videofilters[nb_active_filter-1].filter);
  		 	return;
  		}
  		// Rebuild other filters
                for(uint32_t i=1;i<nb_active_filter;i++)
                {
                    VF_FILTERS tag;
                    AVDMGenericVideoStream *old;
                            old= videofilters[i].filter;
                            tag=videofilters[i].tag;

                            videofilters[i].filter=filterCreateFromTag(tag,
                                                videofilters[i].conf,
                                                videofilters[i-1].filter);
                          delete old;
                }
		aprintf("--preview filter %d\n",nb_active_filter-1);
                ADM_assert(nb_active_filter);
                admPreview::updateFilters(videofilters[0].filter,videofilters[nb_active_filter-1].filter);
}
//
//	Create a filter from : its tag, its config and an input stream
//

AVDMGenericVideoStream *filterCreateFromTag(VF_FILTERS tag,CONFcouple *couple, AVDMGenericVideoStream *in)
{
	 AVDMGenericVideoStream *filter;

			ADM_assert(tag!=VF_INVALID);

                        {
                          for(unsigned int i=0;i<allfilters.size();i++)
                                  {
                                          if(tag==allfilters[i]->tag)
                                                  {
                                                          filter=allfilters[i]->create( in, couple);
                                                          return filter;
                                                  }
                                  }
                        }
			ADM_assert(0);
			return NULL;
}

/*


*/
const FilterDescriptor * filterGetEntryFromTag (VF_FILTERS tag)
{
    ADM_assert(tag!=VF_INVALID);
    for(unsigned int i=0;i<allfilters.size();i++)
    {
        if(tag==allfilters[i]->tag)
        {
            return (allfilters[i]);
        }
    }
    ADM_assert(0);
    return NULL;
}

const char  *filterGetNameFromTag(VF_FILTERS tag)
{
    const FilterDescriptor * entry = filterGetEntryFromTag (tag);
    ADM_assert(entry);
    return entry->name;
}
/*____________________________________
	Save and load current set of filters

	We save/load the tag that identifies the filter
	and a raw hex dump of its config
		+ comment and name in clear text
	____________________________________
*/

/*---------------------------------------*/


uint8_t 	filterAddScript(VF_FILTERS tag,uint32_t n,Arg *args)
{
	// 1- search filter
	int found=-1;
	aviInfo info;

	video_body->getVideoInfo(&info);

	for(unsigned int i=0;i<allfilters.size();i++)
	{
		if(tag==allfilters[i]->tag)
		{
			if(nb_active_filter<1)
			{
  		 		nb_active_filter=1;
  		 		videofilters[0].filter=  new AVDMVideoStreamNull(video_body,0,info.nb_frames);
			}
			AVDMGenericVideoStream *filter=NULL;
			CONFcouple *setup=NULL;
			if(!allfilters[i]->create_from_script)
			{
				printf("That filter cannot be created from script\n");
				return 0;
			}
			filter=allfilters[i]->create_from_script(videofilters[nb_active_filter-1].filter,n-1,&(args[1]));
			if(!filter) return 0;
			videofilters[nb_active_filter].filter=filter;
			videofilters[nb_active_filter].tag=tag;
			filter->getCoupledConf(&setup);
			videofilters[nb_active_filter].conf=setup;
			nb_active_filter++;
			return 1;
		}
	}
	printf("Tag not found:%d\n",tag);
	return 0;
}

void filterSaveScriptJS(FILE *f)
{
                for(int i=1;i<nb_active_filter;i++)
                {
                        VF_FILTERS tag=videofilters[i].tag;
                        qfprintf(f,"app.video.addFilter(");
                        for(unsigned int j=0;j<allfilters.size();j++)
                                {
                                        if(tag==allfilters[j]->tag)
                                        {
                                                qfprintf(f,"\"%s\"",allfilters[j]->filterName);
                                                break;
                                        }
                                }
                        // get args
                        CONFcouple *couple;
                        char *arg,*value,*filtered=NULL;
                        if(videofilters[i].filter->getCoupledConf( &couple))
                        {
                                for(int j=0;j<couple->getNumber();j++)
                                {
                                         couple->getEntry(j, &arg,&value);
                                        // Filter out backslash
                                         filtered=ADM_escape((ADM_filename *)value);
                                         qfprintf(f,",\"%s=%s\"",arg,filtered);
                                         if(filtered) delete [] filtered ;
                                         filtered=NULL;
                                }
                                delete couple;
                        }
                        qfprintf(f,");\n");

                }

}
#endif
// EOF
