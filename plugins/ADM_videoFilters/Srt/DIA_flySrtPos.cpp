/***************************************************************************
                       FlyDialog for Subtitles
    copyright            : (C) 2007 Mean
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
#include <math.h>
#include "DIA_uiTypes.h"
#include "DIA_flyDialog.h"
#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
class ADMfont;
#include "ADM_vidSRT.h"
#include "DIA_flySrtPos.h"
/*********  COMMON PART *********/
uint8_t    flySrtPos::process(void)
{
  uint32_t h;
        // First copy Y
        memcpy(_yuvBufferOut->data,_yuvBuffer->data,(_w*_h));
        // then shift u
        memcpy(UPLANE(_yuvBufferOut),UPLANE(_yuvBuffer),(_w*_h)/4);
        memcpy(VPLANE(_yuvBufferOut),VPLANE(_yuvBuffer),(_w*_h)/4);
        // Mark 
        h=param.fontSize;
        if(h>8) h-=4;
  
        for(uint32_t line=0;line<SRT_MAX_LINE;line++)
        {
                uint8_t *src=YPLANE(_yuvBufferOut)+(param.position+line*param.fontSize)*_w;
  
                for(uint32_t y=0;y<h;y+=2)
                {
                        memset(src,255,_w);
                        src+=2*_w;
                }
        }
        return 1;
}
uint8_t    flySrtPos::update(void)
{
    download();
    process();
	copyYuvFinalToRgb();
    display();
}
//EOF

