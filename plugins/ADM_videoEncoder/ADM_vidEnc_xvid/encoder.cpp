/***************************************************************************
                               xvidEncoder.cpp

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

#include <math.h>

#include "config.h"
#include "ADM_inttype.h"
#include "ADM_files.h"
#include "encoder.h"

static xvidEncoder encoder;
static void* encoders = { &encoder };

extern "C"
{
	void *encoders_getPointer(int uiType) { encoder.setUiType(uiType); return &encoders; } 
	int xvidEncoder_isConfigurable(void) { return encoder.isConfigurable(); }
	int xvidEncoder_configure(vidEncConfigParameters *configParameters, vidEncVideoProperties *properties) { return encoder.configure(configParameters, properties); }
	int xvidEncoder_getOptions(vidEncOptions *encodeOptions, char *pluginOptions, int bufferSize) { return encoder.getOptions(encodeOptions, pluginOptions, bufferSize); };
	int xvidEncoder_setOptions(vidEncOptions *encodeOptions, char *pluginOptions) { return encoder.setOptions(encodeOptions, pluginOptions); };
	int xvidEncoder_getPassCount(void) { return encoder.getPassCount(); }
	int xvidEncoder_getCurrentPass(void) { return encoder.getCurrentPass(); }
	int xvidEncoder_open(vidEncVideoProperties *properties) { return encoder.open(properties); }
	int xvidEncoder_beginPass(vidEncPassParameters *passParameters) { return encoder.beginPass(passParameters); }
	int xvidEncoder_encodeFrame(vidEncEncodeParameters *encodeParams) { return encoder.encodeFrame(encodeParams); }
	int xvidEncoder_finishPass(void) { return encoder.finishPass(); }
	void xvidEncoder_close(void) { encoder.close(); }
}

xvidEncoder::xvidEncoder(void)
{

}

xvidEncoder::~xvidEncoder(void)
{

}

void xvidEncoder::setUiType(int uiType)
{
	_uiType = uiType;
}

int xvidEncoder::isConfigurable(void)
{
	return (_uiType == ADM_UI_GTK || _uiType == ADM_UI_QT4);
}

int xvidEncoder::configure(vidEncConfigParameters *configParameters, vidEncVideoProperties *properties)
{
	if (_loader == NULL)
	{
		char* pluginPath = ADM_getPluginPath();
		const char* configGuiLibName;

		if (_uiType == ADM_UI_GTK)
			configGuiLibName = GTK_PLUGIN_NAME;
		else
			configGuiLibName = QT_PLUGIN_NAME;

		char* configGuiPath = new char[strlen(pluginPath) + 1 + strlen(PLUGIN_SUBDIR) + 1 + strlen(PLUGIN_PREFIX) + strlen(configGuiLibName) + strlen(PLUGIN_SUFFIX) + 1];

		strcpy(configGuiPath, pluginPath);
		strcat(configGuiPath, PLUGIN_SUBDIR);
		strcat(configGuiPath, "/");
		strcat(configGuiPath, PLUGIN_PREFIX);
		strcat(configGuiPath, configGuiLibName);
		strcat(configGuiPath, PLUGIN_SUFFIX);

		_loader = new configGuiLoader(configGuiPath);

		delete [] pluginPath;
		delete [] configGuiPath;
	}

	if (_loader->isAvailable())
		return _loader->showXvidConfigDialog(configParameters, properties, &_encodeOptions, &_options);
	else
		return 0;
}

int xvidEncoder::getOptions(vidEncOptions *encodeOptions, char *pluginOptions, int bufferSize)
{

}

int xvidEncoder::setOptions(vidEncOptions *encodeOptions, char *pluginOptions)
{

}

int xvidEncoder::getCurrentPass(void)
{
	return _currentPass;
}

int xvidEncoder::getPassCount(void)
{
	return _passCount;
}

int xvidEncoder::open(vidEncVideoProperties *properties)
{

}

int xvidEncoder::beginPass(vidEncPassParameters *passParameters)
{

}

int xvidEncoder::encodeFrame(vidEncEncodeParameters *encodeParams)
{

}

int xvidEncoder::finishPass(void)
{

}

void xvidEncoder::close(void)
{

}
