/***************************************************************************
                                 interface.c

    begin                : Wed Jun 11 2008
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
	return "Xvid";
}

const char* vidEncGetEncoderType(int encoderId)
{
	return "MPEG-4 ASP";
}

const char* vidEncGetEncoderDescription(int encoderId)
{
	return "Xvid video encoder plugin for Avidemux (c) Mean/Gruntster";
}

const char* vidEncGetFourCC(int encoderId)
{
	return "xvid";
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
	return "92B544BE-59A3-4720-86F0-6AD5A2526FD2";
}

int vidEncIsConfigurable(int encoderId)
{
	return xvidEncoder_isConfigurable();
}

int vidEncConfigure(int encoderId, vidEncConfigParameters *configParameters, vidEncVideoProperties *properties)
{
	return xvidEncoder_configure(configParameters, properties);
}

int vidEncGetOptions(int encoderId, vidEncOptions *encodeOptions, char *pluginOptions, int bufferSize)
{
	return xvidEncoder_getOptions(encodeOptions, pluginOptions, bufferSize);
}

int vidEncSetOptions(int encoderId, vidEncOptions *encodeOptions, char *pluginOptions)
{
	return xvidEncoder_setOptions(encodeOptions, pluginOptions);
}

int vidEncGetPassCount(int encoderId)
{
	return xvidEncoder_getPassCount();
}

int vidEncGetCurrentPass(int encoderId)
{
	return xvidEncoder_getCurrentPass();
}

int vidEncOpen(int encoderId, vidEncVideoProperties *properties)
{
	return xvidEncoder_open(properties);
}

int vidEncBeginPass(int encoderId, vidEncPassParameters *passParameters)
{
	return xvidEncoder_beginPass(passParameters);
}

int vidEncEncodeFrame(int encoderId, vidEncEncodeParameters *encodeParams)
{
	return xvidEncoder_encodeFrame(encodeParams);
}

int vidEncFinishPass(int encoderId)
{
	return xvidEncoder_finishPass();
}

int vidEncClose(int encoderId)
{
	xvidEncoder_close();

	return ADM_VIDENC_ERR_SUCCESS;
}
