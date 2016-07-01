/**
        \file ADM_pluginLoad.cpp
        \brief Interface for dynamically loaded video filter


*/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "ADM_coreVideoFilterFunc.h"
#include "ADM_dynamicLoading.h"
#include "ADM_videoFilterApi.h"
#include "BVector.h"
#include "config.h"
#include "prefs.h"

#if 1
#define aprintf printf
#else
#define aprintf(...) {}
#endif
static uint32_t lastTag=100;
extern ADM_UI_TYPE UI_GetCurrentUI(void);


#ifdef USE_LIBVA               
#include "ADM_coreVideoCodec/ADM_hwAccel/ADM_coreLibVA/include/ADM_coreLibVA.h"
#endif        
#ifdef USE_VDPAU               
#include "ADM_coreVideoCodec/ADM_hwAccel/ADM_coreVdpau/include/ADM_coreVdpau.h"
#endif        


ADM_vf_plugin::ADM_vf_plugin(const char *file) : ADM_LibWrapper()
{
    initialised = (loadLibrary(file) && getSymbols(11,
        &create, "create",
        &destroy, "destroy",
        &getApiVersion, "getApiVersion",
        &supportedUI, "supportedUI",
        &neededFeatures,"neededFeatures",
        &getFilterVersion, "getFilterVersion",
        &getDesc, "getDesc",
        &getInternalName, "getInternalName",
        &getDisplayName, "getDisplayName",
        &getCategory,"getCategory",
        &partializable,"partializable"
        ));
};


/**
    \fn sortVideoCategoryByName
*/
static bool sortVideoCategoryByName(BVector <ADM_vf_plugin *> &list,const int cat)
{
    int n=list.size();
    for(int start=0;start<n-2;start++)
    for(int i=0;i<n-1;i++)
    {
        ADM_vf_plugin *left=list[i];
        ADM_vf_plugin *right=list[i+1];

        const char       *    leftName=left->getDisplayName();
        const char       *    rightName=right->getDisplayName();
        if(strcasecmp(leftName,rightName)>0)
        {
            list[i]=right;
            list[i+1]=left;
        }
    }
    // rederive tag
    for(int i=0;i<n;i++)
    {
            ADM_vf_plugin *left=list[i];
            left->tag=i+cat*100;
    }
    return true;
}
/**
    \fn sortVideoFiltersByName
*/
static bool sortVideoFiltersByName(void)
{
    for(int i=0;i<VF_MAX;i++)
    {
        sortVideoCategoryByName(ADM_videoFilterPluginsList[i],i);
    }
    return true;

}

/**
 *     \fn tryLoadingVideoFilterPlugin
 *  \brief try to load the plugin given as argument..
 */
static uint8_t tryLoadingVideoFilterPlugin(const char *file,uint32_t featureMask)
{
    ADM_vf_plugin *plugin = new ADM_vf_plugin(file);
    admVideoFilterInfo          *info=NULL;


    if (!plugin->isAvailable())
    {
            printf("[ADM_vf_plugin] Unable to load %s\n", ADM_GetFileName(file));
            goto Err_ad;
    }

    // Check API version
    if (plugin->getApiVersion() != VF_API_VERSION)
    {
            printf("[ADM_vf_plugin] File %s has API version too old (%d vs %d)\n",
                    ADM_GetFileName(file), plugin->getApiVersion(), VF_API_VERSION);
            goto Err_ad;
    }
    if(!(plugin->supportedUI() & UI_GetCurrentUI()))
    {  // FIXME
        ADM_info("==> wrong UI\n");
        goto Err_ad;

    }
    {
        int needed=plugin->neededFeatures();
        if(needed)
        {
            if(  ((needed & featureMask)!=needed))
            {
                ADM_info("[ADM_vf_plugin] %s needs features that are not active\n",plugin->getDisplayName());
                goto Err_ad;
            }
        }
    }
    // Get infos
    uint32_t major, minor, patch;

    plugin->getFilterVersion(&major, &minor, &patch);
    plugin->nameOfLibrary = ADM_strdup(ADM_GetFileName(file));

    info=&(plugin->info);


    info->internalName=plugin->getInternalName();
    info->desc=plugin->getDesc();
    info->displayName=plugin->getDisplayName();
    info->category=plugin->getCategory();

    printf("[ADM_vf_plugin] Plugin loaded version %d.%d.%d, name %s/%s\n",
        major, minor, patch, info->internalName, info->displayName);
    if(info->category>=VF_MAX)
    {
        ADM_error("This filter has an unknown caregory!\n");
        goto Err_ad;

    }else
    {
        plugin->tag=ADM_videoFilterPluginsList[info->category].size()+info->category*100;
        ADM_videoFilterPluginsList[info->category].append(plugin);
    }

    return 1;

Err_ad:
    delete plugin;
    return 0;
}
/**
    \fn ADM_vf_getNbFilters
    \brief returns the # of loaded video filter
*/
uint32_t ADM_vf_getNbFilters(void)
{
    uint32_t sum=0;
    for(int i=0;i<VF_MAX;i++)
    {
        sum+=ADM_videoFilterPluginsList[i].size();
    }
    return sum;
}
/**
    \fn ADM_vf_getNbFiltersInCategory
    \brief returns the # of loaded video filter in the given family
*/
uint32_t ADM_vf_getNbFiltersInCategory(VF_CATEGORY cat)
{
    ADM_assert(cat<VF_MAX);
    return  ADM_videoFilterPluginsList[cat].size();
}
/**
    \fn ADM_vf_getFilterInfo
    \brief returns infos about a given filter
    @param filter [in] # of the filter we are intereseted in, between 0 & ADM_ad_getNbFilters
    @param name [out] Name of the decoder plugin
    @param major, minor,patch [out] Version number
    @return true
*/
bool ADM_vf_getFilterInfo(VF_CATEGORY cat,int filter, const char **name,const char **desc,
                uint32_t *major,uint32_t *minor,uint32_t *patch       )
{

        ADM_assert(filter>=0 && cat<VF_MAX && filter<ADM_videoFilterPluginsList[cat].size());

        ADM_vf_plugin *a=ADM_videoFilterPluginsList[cat][filter];
        a->getFilterVersion(major, minor, patch);

        *name=a->info.displayName;
        *desc=a->info.desc;
        return 1;
}
/**
 * \fn parseOneFolder
 * @param folder
 */
#define MAX_EXTERNAL_FILTER 100
static void parseOneFolder(const char *folder,uint32_t featureMask )
{  
    char *files[MAX_EXTERNAL_FILTER];
    uint32_t nbFile;
    memset(files,0,sizeof(char *)*MAX_EXTERNAL_FILTER);
    if(!buildDirectoryContent(&nbFile, folder, files, MAX_EXTERNAL_FILTER, SHARED_LIB_EXT))
    {
            printf("[ADM_vf_plugin] Cannot parse plugin\n");
            return ;
    }


    ADM_info("Feature Mask = 0x%x\n",featureMask);
    for(int i=0;i<nbFile;i++)
            tryLoadingVideoFilterPlugin(files[i],featureMask);

    printf("[ADM_vf_plugin] Scanning done, found %d video filer(s) so far\n", (int)ADM_vf_getNbFilters());
    clearDirectoryContent(nbFile,files);  
}
/**
 *     \fn ADM_ad_GetPluginVersion
 *  \brief load all audio plugins
 */
uint8_t ADM_vf_loadPlugins(const char *path,const char *subFolder)
{
    
        uint32_t featureMask=0;

#ifdef USE_LIBVA               
        if(admLibVA::isOperationnal()) featureMask|=ADM_FEATURE_LIBVA;
#endif        
#ifdef USE_VDPAU               
        if(admVdpau::isOperationnal()) featureMask|=ADM_FEATURE_VDPAU;
#endif        
#if defined(USE_OPENGL)
        bool hasOpenGl=false;
        prefs->get(FEATURES_ENABLE_OPENGL,&hasOpenGl);
        if(hasOpenGl)
            featureMask|=ADM_FEATURE_OPENGL;
#endif
        
    
    printf("[ADM_vf_plugin] Scanning directory %s\n",path);

    std::string myPath=std::string(path);
    parseOneFolder(myPath.c_str(),featureMask);
    myPath+=std::string("/")+std::string(subFolder);
    parseOneFolder(myPath.c_str(),featureMask);
    
    
    sortVideoFiltersByName();
    return 1;
}
/**
    \fn ADM_vf_cleanup
*/
bool ADM_vf_cleanup(void)
{
    ADM_info("Destroying video filter list\n");
    for(int cat=0;cat<VF_MAX;cat++)
    {
        int nb=ADM_videoFilterPluginsList[cat].size();
        for(int i=0;i<nb;i++)
        {
            ADM_vf_plugin *a=ADM_videoFilterPluginsList[cat][i];
            if(a->nameOfLibrary)
            {
                ADM_dealloc(a->nameOfLibrary);
                a->nameOfLibrary=NULL;
            }
            delete a;
            ADM_videoFilterPluginsList[cat][i]=NULL;
        }
        ADM_videoFilterPluginsList[cat].clear();
    }
    return true;
}

/**
    \fn ADM_vf_getPluginFromTag
    \brief
*/
static ADM_vf_plugin *ADM_vf_getPluginFromInternalName(const char *name)
{
    for(int cat=0;cat<VF_MAX;cat++)
    {
        int nb=ADM_videoFilterPluginsList[cat].size();
        for(int i=0;i<nb;i++)
        {
            if(!strcasecmp(ADM_videoFilterPluginsList[cat][i]->getInternalName(),name))
            {
                return ADM_videoFilterPluginsList[cat][i];
            }
        }
    }
    ADM_error("Cannot get video filter from name %s\n",name);
    ADM_assert(0);
    return NULL;
}
/**
    \fn ADM_VideoFilters
    \brief
*/
const char *ADM_vf_getDisplayNameFromTag(uint32_t tag)
{
    ADM_vf_plugin *plugin=ADM_vf_getPluginFromTag(tag);
    return plugin->info.displayName;
}
/**
    \fn ADM_vf_getFilterCategoryFromTag
*/
VF_CATEGORY ADM_vf_getFilterCategoryFromTag(uint32_t tag)
{
    ADM_vf_plugin *plugin=ADM_vf_getPluginFromTag(tag);
    return plugin->info.category;
}

/**
    \fn ADM_vf_getNameFromTag
    \brief return the uniq name of a filter from the tag , which can vary from one run to another
*/
const char *ADM_vf_getInternalNameFromTag(uint32_t tag)
{
  ADM_vf_plugin *plugin=ADM_vf_getPluginFromTag(tag);
    return plugin->getInternalName();
}
/**
    \fn ADM_vf_getTagFromInternalName
    \brief
*/
uint32_t    ADM_vf_getTagFromInternalName(const char *name)
{
    ADM_vf_plugin *plg=ADM_vf_getPluginFromInternalName(name);
    return plg->tag;

}
bool ADM_vf_canBePartialized(uint32_t tag)
{
  ADM_vf_plugin *plugin=ADM_vf_getPluginFromTag(tag);
  if(!plugin->partializable)
        return false;
  return plugin->partializable();
        
}

//EOF
