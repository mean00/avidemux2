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
uint32_t      ADMVideo_interlaceCount_C( ADMImage *image, int threshold,int skipFactor)

{
    uint32_t m=0;
    int w=image->GetWidth(PLANAR_Y);
    int h=image->GetHeight(PLANAR_Y);
    int pitch=image->GetPitch(PLANAR_Y)<<skipFactor;
    uint8_t *src=image->GetReadPtr(PLANAR_Y);
    
    uint8_t *p,*n,*c;
    int j;
    c=src + pitch;
    n=src + pitch+pitch;
    p=src ;


    for(int y=h>>skipFactor;  y >2 ; y--)
    {
        for(int x=0;x<w;x++)
        {
            j=(int)p[x]-(int)c[x];
            j*=(int)n[x]-(int)c[x];
            if(  j >threshold)
                m++;
        }
        p+=pitch;
        n+=pitch;
        c+=pitch;
    }
    return m;
}

