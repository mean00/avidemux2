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
#include "ADM_vf_plugin.h"
#include "DIA_fileSel.h"
#include "ADM_dynamicLoading.h"
#include <vector>

#if 1
#define aprintf printf
#else
#define aprintf(...) {}
#endif


/**
 *
 */
class ADM_vf_plugin : public ADM_LibWrapper
{
	public:
		ADM_vf_CreateFunction		*create;
		ADM_vf_DeleteFunction		*destroy;
		ADM_vf_SupportedUI		    *supportedUI;
		ADM_vf_GetApiVersion		*getApiVersion;
		ADM_ad_GetPluginVersion	    *getFilterVersion;
		ADM_ADM_vf_GetString    	*getDesc;
        ADM_ADM_vf_GetString    	*getInternalName;
        ADM_ADM_vf_GetString    	*getDisplayName;
        
		const char 					*nameOfLibrary;
        const char                  *internalName;
        const char                  *displayName;
        const char                  *desc;

		ADM_vf_plugin(const char *file) : ADM_LibWrapper()
		{
			initialised = (loadLibrary(file) && getSymbols(6,
				&create, "create",
				&destroy, "destroy",
				&getApiVersion, "getApiVersion",
				&supportedUI, "supportedUI",
				&getFilterVersion, "getFilterVersion",
				&getDesc, "getDesc",
				&getInternalName, "getInternalName",
				&getDisplayName, "getDisplayName"
                ));
		};
};

std::vector<ADM_vf_plugin *> ADM_videoFilterPluginsList;
/**
 * 	\fn tryLoadingVideoFilterPlugin
 *  \brief try to load the plugin given as argument..
 */
static uint8_t tryLoadingVideoFilterPlugin(const char *file)
{
	ADM_vf_plugin *plugin = new ADM_vf_plugin(file);

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

	// Get infos
	uint32_t major, minor, patch;

	plugin->getFilterVersion(&major, &minor, &patch);
	plugin->nameOfLibrary = ADM_strdup(ADM_GetFileName(file));

    plugin->internalName=plugin->getInternalName();
    plugin->desc=plugin->getDesc();
    plugin->displayName=plugin->getDisplayName();

	printf("[ADM_vf_plugin] Plugin loaded version %d.%d.%d, name %s/%s\n",
		major, minor, patch, plugin->internalName, plugin->displayName);

	ADM_videoFilterPluginsList.push_back(plugin);

	return 1;

Err_ad:
	delete plugin;
	return 0;
}
/**
    \fn ADM_ad_GetPluginVersion
    \brief returns the # of loaded audio decoder
*/
uint32_t ADM_vf_getNbFilters(void)
{
    return (uint32_t )ADM_videoFilterPluginsList.size();
}
/**
    \fn ADM_vf_getFilterInfo
    \brief returns infos about a given filter
    @param filter [in] # of the filter we are intereseted in, between 0 & ADM_ad_getNbFilters
    @param name [out] Name of the decoder plugin
    @param major, minor,patch [out] Version number
    @return true
*/
bool ADM_vf_getFilterInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch)
{

        ADM_assert(filter>=0 && filter<ADM_videoFilterPluginsList.size());

    	ADM_vf_plugin *a=ADM_videoFilterPluginsList[filter];
        a->getFilterVersion(major, minor, patch);

        *name=a->displayName;
        return 1;
}

/**
 * 	\fn ADM_ad_GetPluginVersion
 *  \brief load all audio plugins
 */
uint8_t ADM_vf_loadPlugins(const char *path)
{
#define MAX_EXTERNAL_FILTER 50

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
		tryLoadingVideoFilterPlugin(files[i]);

	printf("[ADM_vf_plugin] Scanning done, found %d codec\n", (int)ADM_videoFilterPluginsList.size());

	return 1;
}


//EOF
