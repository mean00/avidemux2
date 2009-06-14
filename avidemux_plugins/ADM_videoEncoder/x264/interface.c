/***************************************************************************
                                 interface.c

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

#include "ADM_inttype.h"
#include "ADM_vidEnc_plugin.h"
#include "encoder.h"

int vidEncGetEncoders(int uiType, int** encoderIds)
{
	if (uiType == ADM_UI_CLI || uiType == ADM_UI_GTK || uiType == ADM_UI_QT4)
	{
		*encoderIds = encoders_getPointer(uiType);
		return ADM_VIDENC_ERR_SUCCESS;
	}
	else
	{
		*encoderIds = NULL;
		return ADM_VIDENC_ERR_FAILED;
	}
}

const char* vidEncGetEncoderName(int encoderId)
{
	return "x264";
}

const char* vidEncGetEncoderType(int encoderId)
{
	return "MPEG-4 AVC";
}

const char* vidEncGetEncoderDescription(int encoderId)
{
	return "x264 video encoder plugin for Avidemux (c) Mean/Gruntster";
}

const char* vidEncGetFourCC(int encoderId)
{
	return "H264";
}

int vidEncGetEncoderRequirements(int encoderId)
{
	return ADM_VIDENC_REQ_NULL_FLUSH;
}

int vidEncGetEncoderApiVersion(int encoderId)
{
	return ADM_VIDENC_API_VERSION;
}

void vidEncGetEncoderVersion(int encoderId, int* major, int* minor, int* patch)
{
	*major = 1;
	*minor = 0;
	*patch = 0;
}

const char* vidEncGetEncoderGuid(int encoderId)
{
	return "32BCB447-21C9-4210-AE9A-4FCE6C8588AE";
}

int vidEncIsConfigurable(int encoderId)
{
	return x264Encoder_isConfigurable();
}

int vidEncConfigure(int encoderId, vidEncConfigParameters *configParameters, vidEncVideoProperties *properties)
{
	return x264Encoder_configure(configParameters, properties);
}

int vidEncGetOptions(int encoderId, vidEncOptions *encodeOptions, char *pluginOptions, int bufferSize)
{
	return x264Encoder_getOptions(encodeOptions, pluginOptions, bufferSize);
}

int vidEncSetOptions(int encoderId, vidEncOptions *encodeOptions, char *pluginOptions)
{
	return x264Encoder_setOptions(encodeOptions, pluginOptions);
}

int vidEncGetPassCount(int encoderId)
{
	return x264Encoder_getPassCount();
}

int vidEncGetCurrentPass(int encoderId)
{
	return x264Encoder_getCurrentPass();
}

int vidEncOpen(int encoderId, vidEncVideoProperties *properties)
{
	return x264Encoder_open(properties);
}

int vidEncBeginPass(int encoderId, vidEncPassParameters *passParameters)
{
	return x264Encoder_beginPass(passParameters);
}

int vidEncEncodeFrame(int encoderId, vidEncEncodeParameters *encodeParams)
{
	return x264Encoder_encodeFrame(encodeParams);
}

int vidEncFinishPass(int encoderId)
{
	return x264Encoder_finishPass();
}

int vidEncClose(int encoderId)
{
	x264Encoder_close();

	return ADM_VIDENC_ERR_SUCCESS;
}
