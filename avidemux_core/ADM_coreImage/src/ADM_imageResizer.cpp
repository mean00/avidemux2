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

#include "ADM_imageResizer.h"

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
    ADM_assert(dest->isWrittable()==true)
    uint32_t srcStride[3];
    uint32_t dstStride[3];
    uint8_t  *srcPtr[3];
    uint8_t  *dstPtr[3];
    source->GetPitches(srcStride);
    dest->GetPitches(dstStride);
    source->GetReadPlanes(srcPtr);
    dest->GetWritePlanes(dstPtr);
    return resizer->convertPlanes(srcStride,dstStride,srcPtr,dstPtr);
}

uint8_t ADMImageResizer::resize(ADMImage *source, uint8_t *dest)
{
    ADM_assert(source->_width == orgWidth);
    ADM_assert(source->_height == orgHeight);
    uint32_t srcStride[3];
    uint32_t dstStride[3];
    uint8_t  *srcPtr[3];
    uint8_t  *dstPtr[3];
    source->GetPitches(srcStride);
    source->GetReadPlanes(srcPtr);
        dstStride[0]=destWidth;
        dstStride[1]=destWidth>>1;
        dstStride[2]=destWidth>>1;
        uint32_t plane=destWidth*destHeight;
        dstPtr[0]=dest;
        dstPtr[1]=dest+plane;
        dstPtr[2]=dest+((5*plane)>>2);

	return resizer->convertPlanes(srcStride,dstStride,srcPtr,dstPtr);
}

uint8_t ADMImageResizer::resize(uint8_t *source, ADMImage *dest)
{
    ADM_assert(dest->_width == destWidth);
    ADM_assert(dest->_height == destHeight);
    ADM_assert(dest->isWrittable()==true)
    uint32_t srcStride[3];
    uint32_t dstStride[3];
    uint8_t  *srcPtr[3];
    uint8_t  *dstPtr[3];
    dest->GetPitches(dstStride);
    dest->GetWritePlanes(dstPtr);
        srcStride[0]=orgWidth;
        srcStride[1]=orgWidth>>1;
        srcStride[2]=orgWidth>>1;
        uint32_t plane=orgWidth*orgHeight;
        srcPtr[0]=source;
        srcPtr[1]=source+plane;
        srcPtr[2]=source+((5*plane)>>2);

	return resizer->convertPlanes(srcStride,dstStride,srcPtr,dstPtr);
}


uint8_t ADMImageResizer::resize(uint8_t *source, uint8_t *dest)
{

	return resizer->convert(source, dest);
}
