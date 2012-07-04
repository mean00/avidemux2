/***************************************************************************
                    
    copyright            : (C) 2006 by mean
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

#include "ADM_image.h"
#include "ADM_print_priv.h"
#define GLYPH_WIDTH  12
#define GLYPH_HEIGHT 20

// Borrowed from decomb
// Borrowed from the author of IT.dll, whose name I
// could not determine.
/**
    \fn drawGlyph
*/
static void drawGlyph(ADMImage *dst, int x, int y, int num,int offset, int color) 
{

	x = x * GLYPH_WIDTH;
	y = y * GLYPH_HEIGHT;
    uint16_t *glyphLine=font[num];
    
	int pitch = dst->GetPitch(PLANAR_Y);
    uint8_t *top=dst->GetWritePtr(PLANAR_Y)+(y+offset)*pitch+(x+4+offset);

    for (int ty = 0; ty < GLYPH_HEIGHT; ty++) 
    {
        uint16_t glyph=glyphLine[ty];
        
        for (int tx = 0; tx < GLYPH_WIDTH; tx++) 
        {
            if(glyph & 0x8000)
                        top[tx]=color;
            glyph<<=1;
        }
        top+=pitch;
    }
}


/**
    \fn printString
*/
bool     ADMImage::printString(uint32_t x,uint32_t y, const char *s)
{
    for (int xx = 0; *s; ++s, ++xx) 
    {
        if(*s==0x0d || *s==0x0a) continue;
        if( (x+xx+1)*GLYPH_WIDTH>_width)
                break;
        drawGlyph(this, x + xx, y, *s - ' ',1,0);
        drawGlyph(this, x + xx, y, *s - ' ',0,0xF0);
    }	

    return true;
}

