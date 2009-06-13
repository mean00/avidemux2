/***************************************************************************
    copyright            : (C) 2003-2005 by mean
    email                : fixounet@free.fr
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

#include "ADM_image.h"

extern "C" {
#include "ADM_ffmpeg/libavcodec/avcodec.h"
#include "ADM_ffmpeg/libswscale/swscale.h"
}


ADMImageResizer::ADMImageResizer(uint32_t ow, uint32_t oh, uint32_t dw, uint32_t dh)
{
	init(ow, oh, dw, dh, PIX_FMT_YUV420P, PIX_FMT_YUV420P);
}

ADMImageResizer::ADMImageResizer(uint32_t ow, uint32_t oh, uint32_t dw, uint32_t dh, int srcFormat, int destFormat)
{
	init(ow, oh, dw, dh, srcFormat, destFormat);
}

ADMImageResizer::~ADMImageResizer()
{
   if (_context)
   {
		sws_freeContext((SwsContext*)_context);
		_context = NULL;
   }
}

void ADMImageResizer::init(uint32_t ow, uint32_t oh, uint32_t dw, uint32_t dh, int srcFormat, int dstFormat)
{
    orgWidth = ow;
    orgHeight = oh;
    destWidth = dw;
    destHeight = dh;

	orgFormat = srcFormat;
	destFormat = dstFormat;

	int flags = SWS_SPLINE;

#ifdef ADM_CPU_X86
	#define ADD(x, y) if (CpuCaps::has##x()) flags |= SWS_CPU_CAPS_##y;

	ADD(MMX,MMX);
	ADD(3DNOW,3DNOW);
	ADD(MMXEXT,MMX2);
#endif

#ifdef ADM_CPU_ALTIVEC
    flags |= SWS_CPU_CAPS_ALTIVEC;
#endif

	_context = (void *)sws_getContext(orgWidth, orgHeight,
									  srcFormat,
									  destWidth, destHeight,
									  destFormat,
									  flags, NULL, NULL,NULL);
}

uint8_t ADMImageResizer::resize(ADMImage *source, ADMImage *dest)
{
    ADM_assert(source->_width == orgWidth);
    ADM_assert(source->_height == orgHeight);
    ADM_assert(dest->_width == destWidth);
    ADM_assert(dest->_height == destHeight);

	return resize(source->data, dest->data);
}

uint8_t ADMImageResizer::resize(ADMImage *source, uint8_t *dest)
{
    ADM_assert(source->_width == orgWidth);
    ADM_assert(source->_height == orgHeight);

	return resize(source->data, dest);
}

uint8_t ADMImageResizer::resize(uint8_t *source, ADMImage *dest)
{
    ADM_assert(dest->_width == destWidth);
    ADM_assert(dest->_height == destHeight);

	return resize(source, dest->data);
}

uint8_t ADMImageResizer::resize(uint8_t *source, uint8_t *dest)
{
	uint8_t *src[3];
	uint8_t *dst[3];
	int ssrc[3];
	int ddst[3];

	if (orgFormat == PIX_FMT_YUV420P)
	{
		getYuvPlanes(source, orgWidth, orgHeight, *&src[0], *&src[1], *&src[2]);

		ssrc[0] = orgWidth;
		ssrc[1] = ssrc[2] = orgWidth >> 1;
	}
	else	// RGB
	{
		src[0] = source;
		src[1] = NULL;
		src[2] = NULL;

		ssrc[0]=orgWidth * 4;
		ssrc[1] = 0;
		ssrc[2] = 0;
	}

	if (destFormat == PIX_FMT_YUV420P)
	{
		getYuvPlanes(dest, destWidth, destHeight, *&dst[0], *&dst[1], *&dst[2]);

		ddst[0] = destWidth;
		ddst[1] = ddst[2] = destWidth >> 1;
	}
	else	// RGB
	{
		dst[0] = dest;
		dst[1] = NULL;
		dst[2] = NULL;

		ddst[0] = destWidth * 4;
		ddst[1] = 0;
		ddst[2] = 0;
	}

    sws_scale((SwsContext*)_context, src, ssrc, 0, orgHeight, dst, ddst);

    return 1;
}

void ADMImageResizer::getYuvPlanes(uint8_t *source, uint32_t width, uint32_t height, uint8_t*& yPlane, uint8_t*& uPlane, uint8_t*& vPlane)
{
	// Unfortunately xPLANE macros expect an ADMImage object so doubling up on logic.
	yPlane = source;
	uPlane = source + (width * height);
	vPlane = source + (5 * (width * height) >> 2);
}
