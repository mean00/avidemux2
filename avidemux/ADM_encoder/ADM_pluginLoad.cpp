/***************************************************************************
                             ADM_pluginLoad.cpp

    begin                : Mon Apr 14 2008
    copyright            : (C) 2008 by gruntster/mean
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
#include "ADM_pluginLoad.h"
#include "ADM_vidEncode.hxx"

struct COMPRES_PARAMS *AllVideoCodec = NULL;
int AllVideoCodecCount = 0;

extern COMPRES_PARAMS *internalVideoCodec[];
extern int getInternalVideoCodecCount();

#if 1
#define aprintf printf
#else
#define aprintf(...) {}
#endif

ADM_vidEnc_plugin::ADM_vidEnc_plugin(const char *file) : ADM_LibWrapper()
{
	initialised = (loadLibrary(file) && getSymbols(20,
		&getEncoders, "vidEncGetEncoders",
		&getEncoderName, "vidEncGetEncoderName",
		&getEncoderType, "vidEncGetEncoderType",
		&getEncoderDescription, "vidEncGetEncoderDescription",
		&getFourCC, "vidEncGetFourCC",
		&getEncoderApiVersion, "vidEncGetEncoderApiVersion",
		&getEncoderVersion, "vidEncGetEncoderVersion",
		&getEncoderGuid, "vidEncGetEncoderGuid",
		&isConfigurable, "vidEncIsConfigurable",
		&configure, "vidEncConfigure",
		&getOptions, "vidEncGetOptions",
		&setOptions, "vidEncSetOptions",
		&getPassCount, "vidEncGetPassCount",
		&getCurrentPass, "vidEncGetCurrentPass",
		&getEncoderGuid, "vidEncGetEncoderGuid",
		&open, "vidEncOpen",
		&beginPass, "vidEncBeginPass",
		&encodeFrame, "vidEncEncodeFrame",
		&finishPass, "vidEncFinishPass",
		&close, "vidEncClose"));
}

std::vector<ADM_vidEnc_plugin *> ADM_videoEncoderPlugins;
/**
    \fn ADM_ve_getNbEncoders
    \brief get the number of encoder plugin loaded
    @return the number of encoder plugins
*/
uint32_t ADM_ve_getNbEncoders(void)
{
    return ADM_videoEncoderPlugins.size();

}
/**
     \fn ADM_ve_getEncoderInfo
     \brief Get info about an encoder plugin
     @param filter [in] Encoder index, between 0 and ADM_ve_getNbEncoders-1 included
     @param name [out] Name + info of the encoder
     @param major,minor,patch [out] Version number of the encoder
     @return true
*/
bool     ADM_ve_getEncoderInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch)
{
    ADM_assert(filter>=0 && filter<ADM_videoEncoderPlugins.size());
    ADM_vidEnc_plugin *plugin =ADM_videoEncoderPlugins[filter];
        *name=plugin->getEncoderDescription(0);
    int ma,mi,pa;
        plugin->getEncoderVersion(0,&ma,&mi,&pa);
        *major=(uint32_t)ma;
        *minor=(uint32_t)mi;
        *patch=(uint32_t)pa;
        return true;

}

static int loadVideoEncoderPlugin(int uiType, const char *file)
{
	ADM_vidEnc_plugin *plugin = new ADM_vidEnc_plugin(file);
	int* encoderIds;
	int encoderCount;
	bool success = false;

	if (plugin->isAvailable())
	{
		// Retrieve video encoders
		encoderCount = plugin->getEncoders(uiType, &encoderIds);

		for (int encoderIndex = 0; encoderIndex < encoderCount; encoderIndex++)
		{
			int encoderId = encoderIds[encoderIndex];
			int apiVersion = plugin->getEncoderApiVersion(encoderId);

			if (apiVersion == ADM_VIDENC_API_VERSION)
			{
				plugin->encoderId = encoderId;
				plugin->fileName = ADM_GetFileName(file);

				int major, minor, patch;

				plugin->getEncoderVersion(encoderId, &major, &minor, &patch);

				printf("[ADM_vidEnc_plugin] Plugin loaded version %d.%d.%d, filename %s, desc: %s\n", major, minor, patch, plugin->fileName, plugin->getEncoderDescription(encoderId));

				ADM_videoEncoderPlugins.push_back(plugin);

				success = true;
			}
			else
				printf("[ADM_vidEnc_plugin] File %s has an outdated API version (%d vs %d)\n", ADM_GetFileName(file), apiVersion, ADM_VIDENC_API_VERSION);
		}
	}
	else
		printf("[ADM_vidEnc_plugin] Unable to load %s\n", ADM_GetFileName(file));

	return success;
}
/**
 *      \fn loadPlugins
 *      \brief load plugin
 */
extern ADM_UI_TYPE UI_GetCurrentUI(void); //FIXME
void loadPlugins(void)
{
        char *pluginDir = ADM_getPluginPath();

        loadVideoEncoderPlugins(UI_GetCurrentUI(), pluginDir);
        delete [] pluginDir;
}

/**
 * 	\fn ADM_vidEnc_loadPlugins
 *  \brief load all audio plugins
 */
int loadVideoEncoderPlugins(int uiType, const char *path)
{
#define MAX_EXTERNAL_FILTER 50


	char *files[MAX_EXTERNAL_FILTER];
	uint32_t nbFile = 0;

	memset(files, 0, sizeof(char *)*MAX_EXTERNAL_FILTER);
	printf("[ADM_vidEnc_plugin] Scanning directory %s\n", path);

	if (!buildDirectoryContent(&nbFile, path, files, MAX_EXTERNAL_FILTER, SHARED_LIB_EXT))
		printf("[ADM_vidEnc_plugin] Cannot parse plugin\n");

	for (int i = 0; i < nbFile; i++)
		loadVideoEncoderPlugin(uiType, files[i]);

	printf("[ADM_vidEnc_plugin] Scanning done, found %"LU" codec\n", ADM_videoEncoderPlugins.size());

	AllVideoCodecCount = ADM_videoEncoderPlugins.size() + getInternalVideoCodecCount();
	AllVideoCodec = new COMPRES_PARAMS[AllVideoCodecCount];

	// Copy over internal codecs
	int internalCodecCount = getInternalVideoCodecCount();

	for (int i = 0; i < internalCodecCount; i++)
		memcpy(&AllVideoCodec[i], internalVideoCodec[i], sizeof(COMPRES_PARAMS));

	// Add external codecs
	for (int i = 0; i < ADM_videoEncoderPlugins.size(); i++)
	{
		ADM_vidEnc_plugin *plugin = ADM_videoEncoderPlugins[i];
		ADM_assert(plugin);

		const char* codecName = plugin->getEncoderName(plugin->encoderId);
		const char* codecType = plugin->getEncoderType(plugin->encoderId);
		char* displayName = new char[strlen(codecName) + strlen(codecType) + 4];

		sprintf(displayName, "%s (%s)", codecType, codecName);

		COMPRES_PARAMS *param = &AllVideoCodec[internalCodecCount + i];

		param->codec = CodecExternal;
		param->menuName = displayName;
		param->tagName = codecName;
		param->extra_param = i;
		param->extraSettings = NULL;
		param->extraSettingsLen = 0;

		int length = plugin->getOptions(plugin->encoderId, NULL, NULL, 0);
		char *pluginOptions = new char[length + 1];
		vidEncOptions encodeOptions;

		plugin->getOptions(plugin->encoderId, &encodeOptions, pluginOptions, length);
		pluginOptions[length] = 0;

		updateCompressionParameters(param, encodeOptions.encodeMode, encodeOptions.encodeModeParameter, pluginOptions, length);
	}

	return 1;
}

void updateCompressionParameters(COMPRES_PARAMS *params, int encodeMode, int encodeModeParameter, char *extraSettings, int extraSettingsLength)
{
	COMPRESSION_MODE compressMode = getCompressionMode(encodeMode);

	params->mode = compressMode;

	switch (compressMode)
	{
		case COMPRESS_CBR:
			params->bitrate = encodeModeParameter;
			break;
		case COMPRESS_AQ:
			params->qz = encodeModeParameter;
			break;
		case COMPRESS_CQ:
			params->qz = encodeModeParameter;
			break;
		case COMPRESS_2PASS:
			params->finalsize = encodeModeParameter;
			break;
		case COMPRESS_2PASS_BITRATE:
			params->avg_bitrate = encodeModeParameter;
			break;
		default:
			ADM_assert(0);
	}

	if (params->extraSettings)
		delete [] (char*)params->extraSettings;

	params->extraSettings = extraSettings;
	params->extraSettingsLen = extraSettingsLength;
}

COMPRESSION_MODE getCompressionMode(int encodeMode)
{
	COMPRESSION_MODE mode;

	switch (encodeMode)
	{
		case ADM_VIDENC_MODE_AQP:
			mode = COMPRESS_AQ;
			break;
		case ADM_VIDENC_MODE_CQP:
			mode = COMPRESS_CQ;
			break;
		case ADM_VIDENC_MODE_CBR:
			mode = COMPRESS_CBR;
			break;
		case ADM_VIDENC_MODE_2PASS_SIZE:
			mode = COMPRESS_2PASS;
			break;
		case ADM_VIDENC_MODE_2PASS_ABR:
			mode = COMPRESS_2PASS_BITRATE;
			break;
		default:
			ADM_assert(0);
	}

	return mode;
}

int getVideoEncodePluginMode(COMPRESSION_MODE mode)
{
	int encodeMode;

	switch (mode)
	{
		case COMPRESS_AQ:
			encodeMode = ADM_VIDENC_MODE_AQP;
			break;
		case COMPRESS_CQ:
			encodeMode = ADM_VIDENC_MODE_CQP;
			break;
		case COMPRESS_CBR:
			encodeMode = ADM_VIDENC_MODE_CBR;
			break;
		case COMPRESS_2PASS:
			encodeMode = ADM_VIDENC_MODE_2PASS_SIZE;
			break;
		case COMPRESS_2PASS_BITRATE:
			encodeMode = ADM_VIDENC_MODE_2PASS_ABR;
			break;
		default:
			ADM_assert(0);
	}

	return encodeMode;
}

ADM_vidEnc_plugin* getVideoEncoderPlugin(int index)
{
	ADM_assert(index < ADM_videoEncoderPlugins.size());

	return ADM_videoEncoderPlugins[index];
}
/**

*/
//EOF
