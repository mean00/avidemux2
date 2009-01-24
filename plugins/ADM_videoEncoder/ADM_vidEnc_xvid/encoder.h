/***************************************************************************
                                  encoder.h

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

#ifndef encoder_h
#define encoder_h

#ifdef __cplusplus
	#include "configGuiLoader.h"

	extern "C"
	{
	#include "ADM_vidEnc_plugin.h"
	}

	class xvidEncoder
	{
	private:
		int _uiType;

		configGuiLoader *_loader;
		xvidOptions _options;
		vidEncOptions _encodeOptions;

		int _currentPass, _passCount;

	public:
		xvidEncoder(void);
		~xvidEncoder(void);
		void setUiType(int uiType);
		int isConfigurable(void);
		int configure(vidEncConfigParameters *configParameters, vidEncVideoProperties *properties);
		int getOptions(vidEncOptions *encodeOptions, char *pluginOptions, int bufferSize);
		int setOptions(vidEncOptions *encodeOptions, char *pluginOptions);
		int getCurrentPass(void);
		int getPassCount(void);
		int open(vidEncVideoProperties *properties);
		int beginPass(vidEncPassParameters *passParameters);
		int encodeFrame(vidEncEncodeParameters *encodeParams);
		int finishPass(void);
		void close(void);
	};
#else
	void *encoders_getPointer(int uiType);
	int xvidEncoder_isConfigurable(void);
	int xvidEncoder_configure(vidEncConfigParameters *configParameters, vidEncVideoProperties *properties);
	int xvidEncoder_getOptions(vidEncOptions *encodeOptions, char *pluginOptions, int bufferSize);
	int xvidEncoder_setOptions(vidEncOptions *encodeOptions, char *pluginOptions);
	int xvidEncoder_getPassCount(void);
	int xvidEncoder_getCurrentPass(void);
	int xvidEncoder_open(vidEncVideoProperties *properties);
	void xvidEncoder_close(void);
	int xvidEncoder_encodeFrame(vidEncEncodeParameters *encodeParams);
#endif	// __cplusplus
#endif	// encoder_h
