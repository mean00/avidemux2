#if 0
/***************************************************************************
                          ADM_mjpegEncode.cpp  -  description
                             -------------------
    begin                : Tue Jul 23 2002
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
#include "ADM_assert.h"
#include <string.h>
#include <math.h>
#include "avi_vars.h"
#include "prototype.h"
#include "config.h"

#ifdef USE_MJPEG
#include "ADM_codecs/ADM_codec.h"
#include "ADM_colorspace/colorspace.h"


extern "C"
{
#include "mjpegtools/jpegutils.h"
}

//#include "ADM_codecs/ADM_divxEncode.h"
#include "ADM_codecs/ADM_mjpegEncode.h"

//
//      The given parameter is the compression ratio of jpeg
//
uint8_t
mjpegEncoder::init (uint32_t val, uint32_t fps1000)
{
  UNUSED_ARG (fps1000);
  _qual = val;
  return 1;
}
uint8_t mjpegEncoder::init (uint32_t val, uint32_t fps1000, uint8_t s)
{
  UNUSED_ARG (fps1000);
  _qual = val;
  _swap = s;
  return 1;
}

uint8_t mjpegEncoder::stopEncoder (void)
{

  return 1;
}
 /*
  * jpeg_data:       buffer with input / output jpeg
  * len:             Length of jpeg buffer
  * itype:           LAV_INTER_NONE: Not interlaced
  *                  LAV_INTER_TOP_FIRST: Interlaced, top-field-first
  *                  LAV_INTER_BOTTOM_FIRST: Interlaced, bottom-field-first
  * ctype            Chroma format for decompression.
  *                  Currently always 420 and hence ignored.
  * raw0             buffer with input / output raw Y channel
  * raw1             buffer with input / output raw U/Cb channel
  * raw2             buffer with input / output raw V/Cr channel
  * width            width of Y channel (width of U/V is width/2)
  * height           height of Y channel (height of U/V is height/2)


  int decode_jpeg_raw (unsigned char *jpeg_data, int len,
  int itype, int ctype, int width, int height,
  unsigned char *raw0, unsigned char *raw1,
  unsigned char *raw2);

  */

uint8_t
  mjpegEncoder::encode (uint8_t * in,
			uint8_t * out, uint32_t * len, uint32_t * flags)
{
  uint8_t *y, *u, *v;
  uint32_t delta;
  int l;

  //
  delta = _w * _h;
  // separate planes
  y = (uint8_t *) in;

  if (!_swap)
    {
      u = y + delta;
      v = u + (delta >> 2);
    }
  else
    {
      v = y + delta;
      u = v + (delta >> 2);
    }

  l = encode_jpeg_raw ((unsigned char *) out, delta,	// insize /2 , should be enough as buffering
		       _qual,	// int quality, ??
		       0, 0,	// ctype /i type
		       _w, _h, y, u, v);
  //      printf("\n size: %d",l);
  *len = l;
  *flags = AVI_KEY_FRAME;

  return 1;
}

uint8_t mjpegEncoder::getResult (void *ress)
{				// for dual pass only
  ADM_assert (0);
  UNUSED_ARG (ress);

}

#endif
#endif
