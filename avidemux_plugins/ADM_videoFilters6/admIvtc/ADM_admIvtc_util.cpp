/***************************************************************************

 Custom IVTC

    copyright            : (C) 2017 by mean
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

#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#define SKIP_FACTOR 2   

// C version
uint32_t      ADMVideo_interlaceCount_C( ADMImage *top,ADMImage *bottom, int threshold,int skipFactor)

{
    uint32_t m=0;
    int w=top->GetWidth(PLANAR_Y);
    int h=top->GetHeight(PLANAR_Y);
    
    int pitchTop=top->GetPitch(PLANAR_Y);
    uint8_t *srcTop=top->GetReadPtr(PLANAR_Y);

    int pitchBottom=bottom->GetPitch(PLANAR_Y);
    uint8_t *srcBottom=bottom->GetReadPtr(PLANAR_Y);

    
    uint8_t *p,*n,*c;
    int j;
    p=srcTop ;
    c=srcBottom + pitchBottom;    
    n=srcTop + pitchTop+pitchTop;
    


    for(int y=h>>(skipFactor+1);  y >2 ; y--)
    {
        for(int x=0;x<w;x++)
        {
            j=(int)p[x]-(int)c[x];
            j*=(int)n[x]-(int)c[x];
            if(  j >threshold)
                m++;
        }
        p+=(2*pitchTop)<<skipFactor;
        n+=(2*pitchTop)<<skipFactor;
        c+=(2*pitchBottom)<<skipFactor;
    }
    return m;
}

