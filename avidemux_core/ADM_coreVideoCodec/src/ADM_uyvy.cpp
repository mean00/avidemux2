/**

    \file ADM_uyvy
    \author mean fixounet@free.fr, 2004-1010
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "fourcc.h"
#include "ADM_uyvy.h"
/**
    \fn uncompress
*/
bool   decoderUYVY::uncompress (ADMCompressedImage * in, ADMImage * out)
{

  if (in->dataLength < _w * _h * 2)
    {
      printf ("in:%d expected%d\n", in->dataLength, _w * _h * 2);
      return 1;
    }
  uint8_t *ptrY, *ptrU, *ptrV, *ptr,*ptr2;
  ptr = in->data;
  ptr2=ptr+2*_w;
  ptrY = YPLANE(out);
  ptrU = VPLANE(out);
  ptrV = UPLANE(out);

  for (uint32_t y = 0; y < _h; y++)
  {
    for (uint32_t x = 0; x < (_w >> 1); x++)
      {
        if (!(y & 1))
          {
            *ptrU++ = ((unsigned int)(ptr[0])+(unsigned int)(ptr2[0]))>>1;
            *ptrV++ = ((unsigned int)(ptr[2])+(unsigned int)(ptr2[2]))>>1;
         }
            *ptrY++ = ptr[1];
            *ptrY++ = ptr[3];
            ptr+=4;
            ptr2+=4;
      }
    }
    out->flags = AVI_KEY_FRAME;
    return 1;

}
/**
    \fn uncompress
*/
bool   decoderYUY2::uncompress  (ADMCompressedImage * in, ADMImage * out)
{

  if (in->dataLength < _w * _h * 2)
    {
      printf ("in:%d expected%d\n", in->dataLength, _w * _h * 2);
      return 1;
    }
  uint8_t *ptrY, *ptrU, *ptrV, *ptr;
  ADM_assert(out->_imageType==ADM_IMAGE_DEFAULT);
  ptr = in->data;
  ptrY = YPLANE(out);
  ptrV = VPLANE(out);
  ptrU = UPLANE(out);
  ADM_assert(_w==out->GetPitch(PLANAR_Y));
  ADM_assert(_w/2==out->GetPitch(PLANAR_U));
  for (uint32_t y = 0; y < _h; y++)
    for (uint32_t x = 0; x < (_w >> 1); x++)
      {
	if (!(y & 1))
	  {
	    *ptrY++ = *ptr++;
	    *ptrU++ = (*(ptr) + *(ptr + _w * 2)) >> 1;
	    ptr++;
	    *ptrY++ = *ptr++;
	    *ptrV++ = (*(ptr) + *(ptr + _w * 2)) >> 1;
	    ptr++;

	  }
	else
	  {
	    *ptrY++ = *(ptr);
	    *ptrY++ = *(ptr + 2);
	    ptr += 4;
	  }
      }


  out->flags = AVI_KEY_FRAME;
  return 1;

}
