/***************************************************************************
                               x264Encoder.h

    begin                : Mon Apr 21 2008
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
	#include "x264.h"
	#include "ADM_vidEnc_plugin.h"
	}

	#define X264_MAX_HEADER_SIZE 1024

	#define H264_NAL_TYPE_SEI       0x6
	#define H264_NAL_TYPE_SEQ_PARAM 0x7
	#define H264_NAL_TYPE_PIC_PARAM 0x8

	class x264Encoder
	{
	private:
		int _uiType;

		configGuiLoader *_loader;
		x264Options _options;
		vidEncOptions _encodeOptions;
		vidEncVideoProperties _properties;

		x264_t *_handle;
		x264_param_t _param;
		x264_picture_t _picture;
		uint8_t *_buffer;
		int _bufferSize;

		uint32_t _currentFrame;
		int _currentPass, _passCount;
		bool _opened, _openPass;

		uint8_t *_seiUserData;
		uint32_t _seiUserDataLen;

		uint8_t *_extraData;
		int _extraDataSize;

		void printParam(x264_param_t* x264Param);
		void printCqm(const uint8_t cqm[], int size);
		void updateEncodeParameters(vidEncVideoProperties *properties);
		unsigned int calculateBitrate(unsigned int fpsNum, unsigned int fpsDen, unsigned int frameCount, unsigned int sizeInMb);
		bool createHeader(void);

	public:
		x264Encoder(void);
		~x264Encoder(void);
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
	int x264Encoder_isConfigurable(void);
	int x264Encoder_configure(vidEncConfigParameters *configParameters, vidEncVideoProperties *properties);
	int x264Encoder_getOptions(vidEncOptions *encodeOptions, char *pluginOptions, int bufferSize);
	int x264Encoder_setOptions(vidEncOptions *encodeOptions, char *pluginOptions);
	int x264Encoder_getPassCount(void);
	int x264Encoder_getCurrentPass(void);
	int x264Encoder_open(vidEncVideoProperties *properties);
	void x264Encoder_close(void);
	int x264Encoder_encodeFrame(vidEncEncodeParameters *encodeParams);
#endif	// __cplusplus
#endif	// encoder_h
