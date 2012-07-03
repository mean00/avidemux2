/**/
/***************************************************************************
                          DIA_flyMpDelogo
                             -------------------

                           Ui for MPlayer DeLogo filter

    begin                : 08 Apr 2005
    copyright            : (C) 2004/5 by mean
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
#include "DIA_flyDialog.h"
#include "ADM_default.h"
#include "ADM_image.h"


#include "delogo.h"
#include "DIA_flyMpDelogo.h"


/************* COMMON PART *********************/
/**
    \fn process
*/
uint8_t    flyMpDelogo::processRgb(uint8_t *imageIn, uint8_t *imageOut)
{
    printf("Process\n");
    uint32_t x,y;
    uint8_t  *in;
    uint32_t w=_w,h=_h;
        
    memcpy(imageOut,imageIn,_w*_h*4);

    int lineTop=param.yoff;
    int lineBottom=param.yoff+param.lh;

    if(lineTop >= _h) lineTop=_h-1;
    if(lineBottom >= _h) lineBottom=_h-1;

    int lineLeft=param.xoff;
    int lineRight=param.xoff+param.lw;

    if(lineLeft >= _w) lineLeft=_w-1;
    if(lineRight >= _w) lineRight=_w-1;

    uint8_t *ptrTop=imageOut+4*_w*lineTop+lineLeft*4;
    uint8_t *ptrBottom=imageOut+4*_w*lineBottom+lineLeft*4;

    int r=lineRight-lineLeft;
    if(!r) r=1;
    for(int x=0;x<r;x++)
    {
            ptrTop[0]=0;
            ptrTop[1]=0xff;
            ptrTop[2]=0;
            ptrTop[3]=0;

            ptrBottom[0]=0;
            ptrBottom[1]=0xff;
            ptrBottom[2]=0;
            ptrBottom[3]=0;
            ptrTop+=4;
            ptrBottom+=4;
    }
    r=lineBottom-lineTop;
    ptrTop=imageOut+4*_w*lineTop+lineLeft*4;
    ptrBottom=imageOut+4*_w*lineTop+lineRight*4;

    if(!r) r=1;
    for(int x=0;x<r;x++)
    {
            ptrTop[0]=0;
            ptrTop[1]=0xff;
            ptrTop[2]=0;
            ptrTop[3]=0;

            ptrBottom[0]=0;
            ptrBottom[1]=0xff;
            ptrBottom[2]=0;
            ptrBottom[3]=0;
            ptrTop+=4*w;
            ptrBottom+=4*w;
    }
  
    return 1;
}
//EOF
