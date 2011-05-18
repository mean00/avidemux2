/***************************************************************************
                          ADM_rgb16.h  -  description
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
#ifndef ADM_RGB16_H
#define ADM_RGB16_H
#include "ADM_codec.h"
#include "ADM_colorspace.h"

class decoderRGB16 : decoders
{
	protected:
		uint8_t       *planar;
		ADMColorScalerSimple *converter;
		bool          isRgb; // Else BGR
		uint32_t      _bpp;
        uint32_t      bytePerPixel;
		uint8_t       *decoded;

	public:
		bool dontcopy (void) { return 1; }
		decoderRGB16 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
		virtual ~decoderRGB16();
		virtual bool uncompress(ADMCompressedImage * in, ADMImage * out);
};

#endif
