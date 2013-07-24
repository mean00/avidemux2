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


static void doLineYOnlybis(int w,uint8_t *src, uint8_t *y, uint8_t *u, uint8_t *v)
{
     for (int x = 0; x < w/2; x++)
      {
	    y[0] = src[1];
            y[1] = src[3];
            y+=2;
            src+=4;
        }    
}
static void doLineYUVbis(int w,uint8_t *src, uint8_t *y, uint8_t *u, uint8_t *v,int stride)
{
     for (int x = 0; x < w/2; x++)
      {
	    y[0] = src[1];
            y[1] = src[3];
            v[0] = (src[0]+src[0+stride])>>1;
            u[0] = (src[2]+src[2+stride])>>1;
            y+=2;
            u++;
            v++;
            src+=4;
        }    
}

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
  int sy=out->GetPitch(PLANAR_Y);
  int su=out->GetPitch(PLANAR_U);
  int sv=out->GetPitch(PLANAR_V);
  for (uint32_t y = 0; y < _h/2; y++)
  {
    doLineYUVbis(_w,ptr,ptrY,ptrU,ptrV,_w*2)  ;
    doLineYOnlybis(_w,ptr+_w*2,ptrY+sy,NULL,NULL)  ;
    ptr+=4*_w;
    ptrY+=2*sy;
    ptrU+=su;
    ptrV+=sv;
  }
    out->Pts=in->demuxerPts;
    out->flags = AVI_KEY_FRAME;
    return 1;

}

static void doLineYOnly(int w,uint8_t *src, uint8_t *y, uint8_t *u, uint8_t *v)
{
     for (int x = 0; x < w/2; x++)
      {
	    y[0] = src[0];
            y[1] = src[2];
            y+=2;
            src+=4;
        }    
}
static void doLineYUV(int w,uint8_t *src, uint8_t *y, uint8_t *u, uint8_t *v,int stride)
{
     for (int x = 0; x < w/2; x++)
      {
	    y[0] = src[0];
            y[1] = src[2];
            v[0] = (src[1]+src[1+stride])>>1;
            u[0] = (src[3]+src[1+stride])>>1;
            y+=2;
            u++;
            v++;
            src+=4;
        }    
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
  int sy=out->GetPitch(PLANAR_Y);
  int su=out->GetPitch(PLANAR_U);
  int sv=out->GetPitch(PLANAR_V);
  for (uint32_t y = 0; y < _h/2; y++)
  {
    doLineYUV(_w,ptr,ptrY,ptrU,ptrV,_w*2)  ;
    doLineYOnly(_w,ptr+_w*2,ptrY+sy,NULL,NULL)  ;
    ptr+=4*_w;
    ptrY+=2*sy;
    ptrU+=su;
    ptrV+=sv;
  }
  out->Pts=in->demuxerPts;
  out->flags = AVI_KEY_FRAME;
  return 1;

}
