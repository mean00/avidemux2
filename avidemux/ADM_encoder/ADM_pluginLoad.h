/***************************************************************************
                              ADM_pluginLoad.h

    begin                : Mon Apr 14 2008
    copyright            : (C) 2008 by gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_pluginLoad_h
#define ADM_pluginLoad_h

#include "ADM_dynamicLoading.h"
#include "ADM_plugin/ADM_vidEnc_plugin.h"
#include "ADM_vidEncode.hxx"

typedef int _vidEncGetEncoders(int uiType, int **encoderIds);
typedef const char* _vidEncGetEncoderName(int encoderId);
typedef const char* _vidEncGetEncoderType(int encoderId);
typedef const char* _vidEncGetEncoderDescription(int encoderId);
typedef const char* _vidEncGetFourCC(int encoderId);
typedef int _vidEncGetEncoderApiVersion(int encoderId);
typedef void _vidEncGetEncoderVersion(int encoderId, int *major, int *minor, int *patch);
typedef const char* _vidEncGetEncoderGuid(int encoderId);

typedef int _vidEncIsConfigurable(int encoderId);
typedef int _vidEncConfigure(int encoderId, vidEncConfigParameters *configParameters, vidEncVideoProperties *properties);
typedef int _vidEncGetOptions(int encoderId, vidEncOptions *encodeOptions, char *pluginOptions, int bufferSize);
typedef int _vidEncSetOptions(int encoderId, vidEncOptions *encodeOptions, char *pluginOptions);

typedef int _vidEncGetPassCount(int encoderId);
typedef int _vidEncGetCurrentPass(int encoderId);

typedef int _vidEncOpen(int encoderId, vidEncVideoProperties *properties);
typedef int _vidEncBeginPass(int encoderId, vidEncPassParameters *passParameters);
typedef int _vidEncEncodeFrame(int encoderId, vidEncEncodeParameters *encodeParams);
typedef int _vidFinishPass(int encoderId);
typedef int _vidEncClose(int encoderId);

class ADM_vidEnc_plugin : public ADM_LibWrapper
{
	public:
		_vidEncGetEncoders *getEncoders;
		_vidEncGetEncoderName *getEncoderName;
		_vidEncGetEncoderType *getEncoderType;
		_vidEncGetEncoderDescription *getEncoderDescription;
		_vidEncGetFourCC *getFourCC;
		_vidEncGetEncoderApiVersion *getEncoderApiVersion;
		_vidEncGetEncoderVersion *getEncoderVersion;
		_vidEncGetEncoderGuid *getEncoderGuid;

		_vidEncIsConfigurable *isConfigurable;
		_vidEncConfigure *configure;
		_vidEncGetOptions *getOptions;
		_vidEncSetOptions *setOptions;

		_vidEncGetPassCount *getPassCount;
		_vidEncGetCurrentPass *getCurrentPass;

		_vidEncOpen *open;
		_vidEncBeginPass *beginPass;
		_vidEncEncodeFrame *encodeFrame;
		_vidFinishPass *finishPass;
		_vidEncClose *close;

		int encoderId;
		const char* fileName;

		ADM_vidEnc_plugin(const char *file);
};

ADM_vidEnc_plugin* getVideoEncoderPlugin(int index);
int loadVideoEncoderPlugins(int uiType, const char *path);
void updateCompressionParameters(COMPRES_PARAMS *params, int encodeMode, int encodeModeParameter, char* extraSettings, int extraSettingsLength);
COMPRESSION_MODE getCompressionMode(int encodeMode);
int getVideoEncodePluginMode(COMPRESSION_MODE mode);

#endif // ADM_pluginLoad_h
