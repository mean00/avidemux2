/***************************************************************************
                          ADM_vidSRT.cpp  -  description
                             -------------------
    begin                : Thu Dec 12 2002
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

#include <math.h>

#include "ADM_default.h"
#include "ADM_vidFont.h"

#define DEFAULT_SIZE  18
#define DEFAULT_WIDTH 16
#define DEFAULT_HEIGHT 20
#define DEFAULT_FONT "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf"

void Error(const char *s)
{
    printf("Error : %s\n",s);
    exit(-1);
}
#define MIN_VALUE 80
static void dump(int c, uint8_t *data,int stride,int w,int h)
{
    data+=stride*2;
    printf("// Char=%c, %d\n",c,c);
    printf(",{\n");
    for(int y=0;y<h;y++)
    {
        int z=0;
        for(int x=0;x<w;x++)
        {
/*
            if(data[x]>MIN_VALUE) printf("*");
                    else printf(".");
*/
            z<<=1;
            if(data[x]>MIN_VALUE) z+=1;
        }
        printf("0x%04x,\n",z);
        data+=stride;

    }
    printf("}\n");
}
int main(int a, char **b)
{
    uint8_t buffer[DEFAULT_HEIGHT*2][DEFAULT_WIDTH*2];
    ADMfont font;
    if(!font.initFreeType(DEFAULT_FONT))
    {
        Error("Cannot initialize font\n");
    }
    font.fontSetSize(DEFAULT_SIZE);
    int w;
    printf("unsigned short font[][20] = {\n");
    for(int i=' ';i<128;i++)
    {
        memset(buffer,0,DEFAULT_HEIGHT*DEFAULT_WIDTH*4);
        font.fontDraw((char *)buffer,i,32, DEFAULT_WIDTH*2,DEFAULT_SIZE,&w);
        printf("//out w=%d\n",w);
        dump(i,(uint8_t *)buffer,DEFAULT_WIDTH*2,DEFAULT_WIDTH,DEFAULT_HEIGHT);
    }
    printf("};\n//EOF\n");
    return 0;
}

// EOF
