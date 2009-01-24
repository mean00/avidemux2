
/***************************************************************************
                          yv.cpp  -  description
                             -------------------

Convert yv422/yuv411 to YV12

Not optimised for speed at all, should take the one from ffmpeg

    begin                : Wed Apr 19 2003
    copyright            : (C) 2003 by mean
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
#include "colorspace.h"

uint8_t COL_422_YV12( uint8_t *in[3], uint32_t stride[3],  uint8_t *out,uint32_t w, uint32_t h)
{


			uint8_t *xsrc;
			uint8_t *dst_y,*dst_u,*dst_v;


			xsrc= in[0];
			dst_y=out;
			// copy y as is
            		for(uint32_t y=h;y>0;y--)
			{
				memcpy(dst_y,xsrc,w);
				dst_y+=w;
				xsrc+=stride[0];
			}
			// u
			xsrc= in[1];
			dst_u=out+(w*h);
			for(uint32_t y=h>>1;y>0;y--)
			{
				memcpy(dst_u,xsrc,w>>1);
				dst_u+=w>>1;
				xsrc+=stride[1]*2;
			}
			// v
			xsrc= in[2];
			dst_v=out+(w*h)+((w*h)>>2);;
			for(uint32_t y=h>>1;y>0;y--)
			{
				memcpy(dst_v,xsrc,w>>1);
				dst_v+=w>>1;
				xsrc+=stride[2]*2;
			}
			return 1;
}
uint8_t COL_411_YV12( uint8_t *in[3], uint32_t stride[3],  uint8_t *out,uint32_t w, uint32_t h)
{


			uint8_t *xsrc;
			uint8_t *dst_y,*dst_u,*dst_v;


			xsrc= in[0];
			dst_y=out;
			// copy y as is
            		for(uint32_t y=h;y>0;y--)
			{
				memcpy(dst_y,xsrc,w);
				dst_y+=w;
				xsrc+=stride[0];
			}
			// u
			xsrc= in[1];
			dst_u=out+(w*h);
			for(uint32_t y=h>>1;y>0;y--)
			{
				//memcpy(dst_u,xsrc,w>>1);
				for(uint32_t x=0;x<w>>2;x++)
					{
						*(dst_u+x+x+1)=*(dst_u+x+x)=*(xsrc+x);
					}
				dst_u+=w>>1;
				xsrc+=stride[1]*2;
			}
			// v
			xsrc= in[2];
			dst_v=out+(w*h)+((w*h)>>2);;
			for(uint32_t y=h>>1;y>0;y--)
			{
				for(uint32_t x=0;x<w>>2;x++)
					{
						*(dst_v+x+x+1)=*(dst_v+x+x)=*(xsrc+x);
					}
				dst_v+=w>>1;
				xsrc+=stride[2]*2;
			}
			return 1;
}


