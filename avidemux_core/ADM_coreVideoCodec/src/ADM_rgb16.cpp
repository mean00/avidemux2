/***************************************************************************
                          ADM_rgb16.cpp  -  description
                             -------------------
    begin                : Mon May 27 2002
    copyright            : (C) 2002 by mean
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ADM_default.h"
//#include "ADM_colorspace/colorspace.h"

#include "ADM_codecs/ADM_codec.h"
#include "ADM_codecs/ADM_rgb16.h"

decoderRGB16::decoderRGB16(uint32_t w, uint32_t h, uint32_t rgb, uint32_t bpp) : decoders (w, h)
{
	isRgb = rgb;
	_bpp = bpp;

	decoded = new uint8_t[_bpp * w * h];
}

decoderRGB16::~decoderRGB16()
{
	delete[] decoded;
}

uint8_t decoderRGB16::uncompress(ADMCompressedImage * in, ADMImage * out)
{
	int xx = _w * _h;
	int lineSize = (_w * (_bpp / 8) + 3) & ~3;
	ADM_colorspace colorspace;
	int i, j;
	uint8_t *src = in->data;
	uint8_t *dst = decoded;

	switch (_bpp)
	{
		case 16:
			// FIXME - 16-bit could use a BGR555 or BGR565 colour mask
			colorspace = ADM_COLOR_BGR555;
			break;
		case 24:
		case 32:
			if(isRgb)
				colorspace = ADM_COLOR_RGB24;
			else
				colorspace = ADM_COLOR_BGR24;

			break;
		default:
			printf("bpp %d not supported\n", _bpp);
			return 0;
	}

	if (_bpp == 32)
	{
		for(i = 0; i < _h; i++)
		{
			uint8_t *buf = src;
			uint8_t *ptr = dst;

			for(j = 0; j < _w; j++)
			{
				ptr[0] = buf[0];
				ptr[1] = buf[1];
				ptr[2] = buf[2];
				ptr += 3;
				buf += 4;
			}

			src += lineSize;
			dst += _w * 3;
		}
	}
	else
	{
		memcpy(decoded, in->data, lineSize * _h);

		if (lineSize == _w * _bpp)
		{
			// no extra junk in scanlines so copy as is
			memcpy(decoded, in->data, lineSize * _h);
		}
		else
		{
			// strip extra junk from scanlines (due to 4 byte alignment)
			for(i = 0; i < _h; i++)
			{
				memcpy(dst, src, _w * (_bpp / 8));
				src += lineSize;
				dst += _w * (_bpp / 8);
			}
		}
	}

	ADM_assert(out->_isRef);

	out->flags = AVI_KEY_FRAME;
	out->_colorspace = (ADM_colorspace)(colorspace | ADM_COLOR_BACKWARD);

	out->_planes[0] = decoded;
	out->_planes[1] = NULL;
	out->_planes[2] = NULL;

	out->_planeStride[0] = (_bpp / 8) * _w;
	out->_planeStride[1] = 0;
	out->_planeStride[2] = 0;

	return 1;
}
//EOF
