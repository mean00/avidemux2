/**
        \file ADM_pluginLoad.cpp
        \brief Interface for dynamically loaded audio decoder

        There are 2 known problem here
        1: The destructor is called instead of calling destroy in the class factory
        2: Memory leak, ADM_audioPlugins is not destroyed as of today

*/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_ad_plugin.h"
#include "DIA_fileSel.h"
#include "ADM_dynamicLoading.h"
#include <vector>

#if 1
#define aprintf printf
#else
#define aprintf(...) {}
#endif
/*  Exported functions */
uint32_t ADM_ad_getNbFilters(void);
bool     ADM_ad_getFilterInfo(int filter, const char **name,
                                uint32_t *major,uint32_t *minor,uint32_t *patch);

/**
 *
 */
class ADM_ad_plugin : public ADM_LibWrapper
{
	public:
		ADM_ad_CreateFunction		*create;
		ADM_ad_DeleteFunction		*destroy;
		ADM_ad_SupportedFormat		*supportedFormat;
		ADM_ad_GetApiVersion		*getApiVersion;
		ADM_ad_GetDecoderVersion	*getDecoderVersion;
		ADM_ADM_ad_GetInfo			*getInfo;
		const char 					*name;

		ADM_ad_plugin(const char *file) : ADM_LibWrapper()
		{
			initialised = (loadLibrary(file) && getSymbols(6,
				&create, "create",
				&destroy, "destroy",
				&supportedFormat, "supportedFormat",
				&getApiVersion, "getApiVersion",
				&getDecoderVersion, "getDecoderVersion",
				&getInfo, "getInfo"));
		};
};

std::vector<ADM_ad_plugin *> ADM_audioPlugins;
/**
 * 	\fn tryLoadingAudioPlugin
 *  \brief try to load the plugin given as argument..
 */
static uint8_t tryLoadingAudioPlugin(const char *file)
{
	ADM_ad_plugin *plugin = new ADM_ad_plugin(file);

	if (!plugin->isAvailable())
	{
		printf("[ADM_ad_plugin] Unable to load %s\n", ADM_GetFileName(file));
		goto Err_ad;
	}

	// Check API version
	if (plugin->getApiVersion() != AD_API_VERSION)
	{
		printf("[ADM_ad_plugin] File %s has API version too old (%d vs %d)\n",
			ADM_GetFileName(file), plugin->getApiVersion(), AD_API_VERSION);
		goto Err_ad;
	}

	// Get infos
	uint32_t major, minor, patch;

	plugin->getDecoderVersion(&major, &minor, &patch);
	plugin->name = ADM_strdup(ADM_GetFileName(file));

	printf("[ADM_ad_plugin] Plugin loaded version %d.%d.%d, name %s, desc: %s\n",
		major, minor, patch, plugin->name, plugin->getInfo());

	ADM_audioPlugins.push_back(plugin);

	return 1;

Err_ad:
	delete plugin;
	return 0;
}
/**
    \fn ADM_ad_getNbFilters
    \brief returns the # of loaded audio decoder
*/
uint32_t ADM_ad_getNbFilters(void)
{
    return (uint32_t )ADM_audioPlugins.size();
}
/**
    \fn ADM_ad_getFilterInfo
    \brief returns infos about a given filter
    @param filter [in] # of the filter we are intereseted in, between 0 & ADM_ad_getNbFilters
    @param name [out] Name of the decoder plugin
    @param major, minor,patch [out] Version number
    @return true
*/
bool ADM_ad_getFilterInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch)
{

        ADM_assert(filter>=0 && filter<ADM_audioPlugins.size());

    	ADM_ad_plugin *a=ADM_audioPlugins[filter];
        a->getDecoderVersion(major, minor, patch);

        *name=a->getInfo();
        return 1;
}

/**
 * 	\fn ADM_ad_loadPlugins
 *  \brief load all audio plugins
 */
uint8_t ADM_ad_loadPlugins(const char *path)
{
#define MAX_EXTERNAL_FILTER 50

	char *files[MAX_EXTERNAL_FILTER];
	uint32_t nbFile;

	memset(files,0,sizeof(char *)*MAX_EXTERNAL_FILTER);
	printf("[ADM_ad_plugin] Scanning directory %s\n",path);

	if(!buildDirectoryContent(&nbFile, path, files, MAX_EXTERNAL_FILTER, SHARED_LIB_EXT))
	{
		printf("[ADM_ad_plugin] Cannot parse plugin\n");
		return 0;
	}

	for(int i=0;i<nbFile;i++)
		tryLoadingAudioPlugin(files[i]);

	printf("[ADM_ad_plugin] Scanning done, found %d codec\n", (int)ADM_audioPlugins.size());

	return 1;
}
/**
 * 	\fn ADM_ad_searchCodec
 *  \brief Try to instantiate a codec that can decode the given stuff
 */
ADM_Audiocodec *ADM_ad_searchCodec(uint32_t fourcc,	WAVHeader *info,uint32_t extraLength,uint8_t *extraData)
{
	for(int i=0;i<ADM_audioPlugins.size();i++)
	{
		ADM_ad_plugin *a=ADM_audioPlugins[i];
		ADM_assert(a);
		ADM_assert(a->supportedFormat);
		aprintf("[ADM_ad_plugin]Format 0x%x : probing %s\n",fourcc,a->name);
		if(a->supportedFormat(fourcc)==true)
		{
			ADM_assert(a->create);
			return a->create(fourcc, info,extraLength,extraData);
		}
	}
	return NULL;
}

//EOF
