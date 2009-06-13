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

// Borrowed from decomb
// Borrowed from the author of IT.dll, whose name I
// could not determine.

void drawDigit(ADMImage *dst, int x, int y, int num) 
{

	x = x * 10;
	y = y * 20;

	int pitch = dst->_width;
	for (int tx = 0; tx < 10; tx++) {
		for (int ty = 0; ty < 20; ty++) {
			unsigned char *dp = YPLANE(dst);
                        dp+=(y+ty)*pitch+(x+tx)*2;
//&dst->GetWritePtr()[(x + tx) * 2 + (y + ty) * pitch];
			if (font[num][ty] & (1 << (15 - tx))) {
				if (tx & 1) {
					dp[0] = 250;
					dp[-1] = 128;
					dp[1] = 128;
				} else {
					dp[0] = 250;
					dp[1] = 128;
					dp[3] = 128;
				}
			} else {
				if (tx & 1) {
					dp[0] = (unsigned char) ((dp[0] * 3) >> 2);
					dp[-1] = (unsigned char) ((dp[-1] + 128) >> 1);
					dp[1] = (unsigned char) ((dp[1] + 128) >> 1);
				} else {
					dp[0] = (unsigned char) ((dp[0] * 3) >> 2);
					dp[1] = (unsigned char) ((dp[1] + 128) >> 1);
					dp[3] = (unsigned char) ((dp[3] + 128) >> 1);
				}
			}
		}
	}
}
static void drawDigitSmall(ADMImage *dst, int x, int y, int num) 
{

	x = x * 6;
	y = y * 20;

	int pitch = dst->_width;
	for (int tx = 0; tx < 10; tx++) {
		for (int ty = 0; ty < 20; ty++) 
		{
			unsigned char *dp = YPLANE(dst);

			dp+=(y+ty)*pitch+(x+tx)*2;

			if (font[num][ty] & (1 << (15 - tx))) 
			{
				
					dp[0] = 250;
			} else 
			{
					dp[0] = (unsigned char) ((dp[0] * 3) >> 2);
						
			}
		}
	}
}


void drawString(ADMImage *dst, int x, int y, const char *s) 
{
	int len=strlen(s);
	if( ((x+len)*20)<dst->_width)
	{
		for (int xx = 0; *s; ++s, ++xx) 
			{
				if(*s==0x0d || *s==0x0a) continue;
				drawDigit(dst, x + xx, y, *s - ' ');
			}	
	}
	else
	{
		for (int xx = 0; *s; ++s, ++xx) 
				{
					if(*s==0x0d || *s==0x0a) continue;
					drawDigitSmall(dst, x + xx, y, *s - ' ');
				}	
	}
	
}

