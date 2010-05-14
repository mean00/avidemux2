//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
//	This is the base time for image exchanged between codec/filters/...
//
//	We (optionnally) can carry extra informations
//		- aspect ratio
//		- frame type
//		- quantizer for each macroblock (16x16 pixels)
//      - PTS : Presentation time in us of the image
//	For the latter 3 infos are used
//		quant which leads to the int8 quant array
//		qstride = stride of array. Usually width+15)/16. 0 MEANS NOT USABLE
//		qsize = size of the array (needed to be able to copy it)
//
#ifndef ADM_IMAGE_RESIZER_H
#define ADM_IMAGE_RESIZER_H
#include "ADM_image.h"
/**
        \class ADMImageResizer
        \brief Simple image resizer
*/
class ADMImageResizer
{
	private:
		ADMColorScalerFull   *resizer;
        ADM_colorspace orgFormat, destFormat;
		uint32_t orgWidth, orgHeight;
		uint32_t destWidth, destHeight;
        void        init(uint32_t ow, uint32_t oh, uint32_t dw, uint32_t dh, ADM_colorspace srcFormat, ADM_colorspace dstFormat);
	public:
		ADMImageResizer(uint32_t ow,uint32_t oh, uint32_t dw, uint32_t dh);
		ADMImageResizer(uint32_t ow, uint32_t oh, uint32_t dw, uint32_t dh, ADM_colorspace srcFormat, ADM_colorspace dstFormat);
		~ADMImageResizer();
		
		uint8_t resize(ADMImage *src, ADMImage *dest);
		uint8_t resize(uint8_t *src, ADMImage *dest);
		uint8_t resize(ADMImage *src, uint8_t *dest);
		uint8_t resize(uint8_t *src, uint8_t *dest);
};

#endif
