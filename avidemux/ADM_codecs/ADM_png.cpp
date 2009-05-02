/***************************************************************************
                          ADM_png.cpp  -  description
                             -------------------
    begin                : Sat Sep 28 2002
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
#include "config.h"
#ifdef USE_PNG
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>

#include "ADM_default.h"

//#include "ADM_colorspace/colorspace.h"

#include "ADM_codecs/ADM_codec.h"
#include "ADM_codecs/ADM_png.h"
extern "C"
{
#include "/usr/include/png.h"
}
#include "ADM_assert.h"

#define PNG_PTR ((png_structp)png_ptr)
#define INFO_PTR ((png_infop)info_ptr)
#define INFO_END ((png_infop)end_info)
/*
   	Initialize codec
*/
static void user_read_data (png_structp png_ptr, png_bytep data, png_size_t length);

void decoderPng::recalc (void)
{
  int mul;
  if (colorspace == ADM_COLOR_RGB24)
    mul = 3;

  else
    mul = 4;
  for (int i = 0; i < _h; i++)
    rows[i] = decoded + mul * _w * i;
}


decoderPng::decoderPng (uint32_t w, uint32_t h):decoders (w, h)
{
  rows = NULL;
  decoded = NULL;
  colorspace = ADM_COLOR_RGB24;

  //****************************
  // Prepare the decoded buffer*
  //****************************
  decoded = new uint8_t[4 * w * h];	// We take a bit more to be able to decode 32 bits png
  // without causing a segfault
  rows = new uint8_t *[h];
  recalc ();
}
 void decoderPng::Init (void)
{
  png_ptr =
    (void *) png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  ADM_assert (png_ptr);
  info_ptr = (void *) png_create_info_struct (PNG_PTR);
  ADM_assert (info_ptr);
  end_info = (void *) png_create_info_struct (PNG_PTR);
  ADM_assert (end_info);
  memset (&io, 0, sizeof (io));
  png_set_read_fn (PNG_PTR, &io, user_read_data);
  png_set_rows (PNG_PTR, INFO_PTR, (png_byte **) rows);
}
void decoderPng::Cleanup (void)
{
  png_destroy_read_struct ((png_structpp) & png_ptr, (png_infopp) & info_ptr,
			   (png_infopp) & end_info);
}
decoderPng::~decoderPng ()
{
  delete[]rows;
  delete[]decoded;
}
/*
   	Uncompress frame, set flags if needed
*/
uint8_t decoderPng::uncompress(ADMCompressedImage * in, ADMImage * out)
{
  int bpp;
  int colortype;

  // Check if it is png, and fill it
  if (!!png_sig_cmp (in->data, 0, 8))

    {
      printf ("[PNG] wrong sig\n");
      return 0;
    }

  //
  //
gain2:
  Init ();
  io.data = in->data;
  io.size = in->dataLength;
  io.cur = 0;
  png_read_png (PNG_PTR, INFO_PTR, PNG_TRANSFORM_IDENTITY, NULL);

  // Check if it is 24 or 32 bits RGB 
  bpp = png_get_bit_depth (PNG_PTR, INFO_PTR);
//   printf("Bpp:%u\n",bpp);
  // if needed we change colorspace 
  colortype = png_get_color_type (PNG_PTR, INFO_PTR);
  // 
  if (colorspace == ADM_COLOR_RGB24 && colortype == PNG_COLOR_TYPE_RGB_ALPHA)	// RGB32
    {

      // Switch to 32 bits
      colorspace = ADM_COLOR_RGB32A;
      recalc ();
      goto gain2;
    }

  else if (colorspace == ADM_COLOR_RGB32A && colortype == PNG_COLOR_TYPE_RGB)

    {

      // Switch to 24 bits
      colorspace = ADM_COLOR_RGB24;
      recalc ();
      goto gain2;
    }
  ADM_assert (out->_isRef);
  out->_planes[0] = decoded;
  out->_planes[1] = NULL;
  out->_planes[2] = NULL;
  if (colorspace == ADM_COLOR_RGB32A)
    out->_planeStride[0] = _w * 4;

  else
    out->_planeStride[0] = _w * 3;
  out->_planeStride[1] = 0;
  out->_planeStride[2] = 0;
  out->_colorspace = colorspace;
  Cleanup ();
  return 1;
}

// ******************************************************
//    Memory based IO
// ******************************************************
void user_read_data (png_structp png_ptr, png_bytep data, png_size_t length)
{
  memAccess *ac;
  ac = (memAccess *) png_get_io_ptr (png_ptr);
  ADM_assert (length + ac->cur <= ac->size);	// or < ?
  memcpy (data, ac->data + ac->cur, length);
  ac->cur += length;
} 

void user_write_data (png_structp png_ptr, png_bytep data, png_size_t length)
{
} 

void user_flush_data (png_structp png_ptr)
{
}
//EOF
#endif /*  */
