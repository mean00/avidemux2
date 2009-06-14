/***************************************************************************
                          ADM_resizebis.hxx  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#ifndef __RESIZE_B_SUPPORT
#define __RESIZE_B_SUPPORT

typedef struct {
    double (*f) (double x);
    double support;
} ResampleFunc;

INT *GetResamplingPattern(uint32_t original_width,
			      uint32_t target_width, ResampleFunc * func);

INT *GetResamplingPatternFIR4(uint32_t original_width,
			      uint32_t target_width, ResampleFunc * func);
		
static inline uint8_t PixelClip(int16_t in)
{
    if (in > 255)
	in = 255;
   if(in<0)
   	in=0;
    return (uint8_t) in;
};
			   	   
static inline unsigned char ScaledPixelClip(INT i)
{
    return PixelClip(((i +32768) >> 16)); //  + 32768
};

static inline unsigned char ScaledPixelClip8(int16_t i)
{
    return PixelClip(((i +256) >> 8)); //  + 32768
};

			         
#endif
