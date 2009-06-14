#if 0
/***************************************************************************
                          ADM_mjpeg.cpp  -  description
                             -------------------
          I think i could use plain jpeg instead but....


    begin                : Fri Apr 12 2002
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
#include "ADM_default.h"
#include "config.h"
#ifdef USE_MJPEG
#include "ADM_colorspace/colorspace.h"

#include "ADM_codecs/ADM_codec.h"
#include "ADM_codecs/ADM_mjpeg.h"
extern "C"
{
#include "mjpegtools/jpegutils.h"
}
#include "ADM_gui/GUI_decodersettings.h"
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
void
decoderMjpeg::setParam (void)
{
  int param;

  param = _swap;
  if (1 == getMjpegParams (&param))
    {
      _swap = param;
    }

}
//________________________________________________

uint8_t
  decoderMjpeg::uncompress (uint8_t * in, uint8_t * out, uint32_t len,
			    uint32_t * flagz)
{
  //
  uint32_t delta;
  uint8_t *outu, *outv;
  //
  UNUSED_ARG (flagz);


  delta = _w * _h;

  outu = out + delta;
  outv = outu + (delta >> 2);
  if (!_swap)

    decode_jpeg_raw (in, len, 1, 0, _w, _h, out, outu, outv);

  else
    decode_jpeg_raw (in, len, 1, 0, _w, _h, out, outv, outu);

  return 1;


}

//_____________________________________________________

decoderMjpeg::~decoderMjpeg ()
{


}
// constructor for mjpeg, init encoder and stuff
decoderMjpeg::decoderMjpeg (uint32_t w, uint32_t h):decoders (w, h)
{
  // some mjpeg are encoded with u & v inverted
  _swap = 0;

}
#endif
#endif
