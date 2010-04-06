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
	init(ow, oh, dw, dh, ADM_COLOR_YV12, ADM_COLOR_YV12);
}

ADMImageResizer::ADMImageResizer(uint32_t ow, uint32_t oh, uint32_t dw, uint32_t dh,ADM_colorspace srcFormat, ADM_colorspace destFormat)
{
	init(ow, oh, dw, dh, srcFormat, destFormat);
}

ADMImageResizer::~ADMImageResizer()
{
   if (resizer)
   {
		delete resizer;
		resizer = NULL;
   }
}

void ADMImageResizer::init(uint32_t ow, uint32_t oh, uint32_t dw, uint32_t dh, ADM_colorspace srcFormat, ADM_colorspace dstFormat)
{
    orgWidth = ow;
    orgHeight = oh;
    destWidth = dw;
    destHeight = dh;

	orgFormat = srcFormat;
	destFormat = dstFormat;
    resizer=new ADMColorScalerFull(ADM_CS_BICUBIC,orgWidth,orgHeight,destWidth,destHeight,srcFormat,destFormat);
                        
}

uint8_t ADMImageResizer::resize(ADMImage *source, ADMImage *dest)
{
    ADM_assert(source->_width == orgWidth);
    ADM_assert(source->_height == orgHeight);
    ADM_assert(dest->_width == destWidth);
    ADM_assert(dest->_height == destHeight);

	return resizer->convert(source->data, dest->data);
}

uint8_t ADMImageResizer::resize(ADMImage *source, uint8_t *dest)
{
    ADM_assert(source->_width == orgWidth);
    ADM_assert(source->_height == orgHeight);

	return resizer->convert(source->data, dest);
}

uint8_t ADMImageResizer::resize(uint8_t *source, ADMImage *dest)
{
    ADM_assert(dest->_width == destWidth);
    ADM_assert(dest->_height == destHeight);

	return resizer->convert(source, dest->data);
}


uint8_t ADMImageResizer::resize(uint8_t *source, uint8_t *dest)
{

	return resizer->convert(source, dest);
}
