/***************************************************************************
                          ADM_externalEncoder.cpp

    begin                : Tue Apr 15 2008
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

#include "ADM_default.h"
#include "ADM_externalEncoder.h"
#include "ADM_plugin/ADM_vidEnc_plugin.h"


externalEncoder::externalEncoder(COMPRES_PARAMS *params, bool globalHeader)
{
	_plugin = getVideoEncoderPlugin(params->extra_param);
	_openPass = false;
	_logFileName = NULL;
	_globalHeader = globalHeader;
}

externalEncoder::~externalEncoder()
{
	stop();

	if (_logFileName)
		delete [] _logFileName;
}

uint8_t externalEncoder::configure(AVDMGenericVideoStream *instream, int useExistingLogFile)
{
	ADV_Info *info;

	info = instream->getInfo();
	_w = info->width;
	_h = info->height;

	_vbuffer = new ADMImage (_w, _h);
	ADM_assert(_vbuffer);

	_in = instream;

	vidEncVideoProperties properties;

	memset(&properties, 0, sizeof(vidEncVideoProperties));

	properties.structSize = sizeof(vidEncVideoProperties);
	properties.width = _w;
	properties.height = _h;
	properties.parWidth = instream->getPARWidth();
	properties.parHeight = instream->getPARHeight();
	properties.frameCount = info->nb_frames;
	properties.fps1000 = info->fps1000;
	properties.logFileName = _logFileName;
	properties.useExistingLogFile = useExistingLogFile;

	if (_globalHeader)
		properties.flags |= ADM_VIDENC_FLAG_GLOBAL_HEADER;

	if (_plugin->open(_plugin->encoderId, &properties))
	{
		if (!isDualPass())
			return (startPass() == ADM_VIDENC_ERR_SUCCESS);
		else
			return 1;
	}
	else
		return 0;
}

uint8_t externalEncoder::encode(uint32_t frame, ADMBitstream *out)
{
	uint32_t l, f;
	int outLength, videoFrameType;
	int64_t ptsFrame;
	vidEncEncodeParameters params;

	if (!_in->getFrameNumberNoAlloc(frame, &l, _vbuffer, &f))
	{
		printf ("[externalEncoder] Error reading incoming frame\n");
		return 0;
	}

	params.structSize = sizeof(vidEncEncodeParameters);
	params.frameData = _vbuffer->data;
	params.frameDataSize = ((_w + 15) & 0xffffff0) * ((_h + 15) & 0xfffffff0) * 2;
	params.encodedData = NULL;
	params.encodedDataSize = 0;

	if (_plugin->encodeFrame(_plugin->encoderId, &params))
	{
		memcpy(out->data, params.encodedData, params.encodedDataSize);

		out->len = params.encodedDataSize;
		out->ptsFrame = params.ptsFrame;
		out->out_quantizer = params.quantiser;

		switch (params.frameType)
		{
			case ADM_VIDENC_FRAMETYPE_IDR:
				out->flags = AVI_KEY_FRAME;
				break;
			case ADM_VIDENC_FRAMETYPE_B:
				out->flags = AVI_B_FRAME;
				break;
			case ADM_VIDENC_FRAMETYPE_P:
				out->flags = AVI_P_FRAME;
				break;
			default:
				assert(0);
				break;
		}

		return 1;
	}
	else
		return 0;
}

const char* externalEncoder::getCodecName(void)
{
	return _plugin->getEncoderName(_plugin->encoderId);
}

const char* externalEncoder::getDisplayName(void)
{
	return _plugin->getEncoderName(_plugin->encoderId);
}

const char* externalEncoder::getFCCHandler(void)
{
	return _plugin->getFourCC(_plugin->encoderId);
}

uint8_t externalEncoder::isDualPass(void)
{
	return _plugin->getPassCount(_plugin->encoderId) == 2;
}

uint8_t externalEncoder::startPass(void)
{
	vidEncPassParameters passParameters;

	memset(&passParameters, 0, sizeof(vidEncPassParameters));
	passParameters.structSize = sizeof(vidEncPassParameters);

	_openPass = (_plugin->beginPass(_plugin->encoderId, &passParameters) == ADM_VIDENC_ERR_SUCCESS);

	if (_openPass && _globalHeader)
	{
		_extraData = passParameters.extraData;
		_extraDataSize = passParameters.extraDataSize;
	}
	else
	{
		_extraData = NULL;
		_extraDataSize = 0;
	}

	return _openPass;
}

uint8_t externalEncoder::startPass1(void)
{
	return startPass();
}

uint8_t externalEncoder::startPass2(void)
{
	if (_openPass)
		_plugin->finishPass(_plugin->encoderId);

	return startPass();
}

uint8_t externalEncoder::hasExtraHeaderData(uint32_t *l, uint8_t **data)
{
	printf ("[externalEncoder] %d extra header bytes\n", _extraDataSize);

	if (_globalHeader && _extraDataSize > 0)
	{
		*data = _extraData;
		*l = _extraDataSize;

		return 1;
	}
	else
		return 0;
}

uint8_t externalEncoder::stop(void)
{
	if (_openPass)
		_plugin->finishPass(_plugin->encoderId);

	_plugin->close(_plugin->encoderId);

	return 1;
}

uint8_t externalEncoder::setLogFile(const char *p, uint32_t fr)
{
	if (_logFileName)
		delete [] _logFileName;

	_logFileName = new char[strlen(p) + 1];
	strcpy(_logFileName, p);

	return 1;
}
