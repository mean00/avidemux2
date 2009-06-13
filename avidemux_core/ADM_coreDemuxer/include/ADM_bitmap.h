/***************************************************************************
                          bitmap.h  -  description
                             -------------------
    begin                : Sat Dec 22 2001
    copyright            : (C) 2001 by mean
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

#ifndef ADM_BITMAP_H
#define ADM_BITMAP_H

typedef struct
{
    uint32_t 	biSize;
    uint32_t  	biWidth;
    uint32_t  	biHeight;
    uint16_t 	biPlanes;
    uint16_t 	biBitCount;
    uint32_t 	biCompression;
    uint32_t 	biSizeImage;
    uint32_t  	biXPelsPerMeter;
    uint32_t  	biYPelsPerMeter;
    uint32_t 	biClrUsed;
    uint32_t 	biClrImportant;
} ADM_BITMAPINFOHEADER;

typedef struct 
{
        ADM_BITMAPINFOHEADER bmiHeader;
        int	bmiColors[1];
} ADM_BITMAPINFO;


typedef struct 
{
   uint16_t    bfType;
   uint32_t    bfSize;
   int16_t     bfReserved1;
   int16_t     bfReserved2;
   uint32_t    bfOffBits;
}  ADM_BITMAPFILEHEADER;

#endif
