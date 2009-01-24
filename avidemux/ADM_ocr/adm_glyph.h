/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_GLYPH_
#define ADM_GLYPH_

#include "ADM_default.h"
/*
    returns 1 if the line is empty
    0 if not
*/
static inline uint8_t lineEmpty(uint8_t *base, uint32_t stride, uint32_t width, uint32_t line)
{
    base+=line*stride;
    for(uint32_t x=0;x<width;x++)
    {
        if(base[x]) return 0;
    }
    return 1;
}
/*
    returns 1 if the line is empty
    0 if not
*/
static inline uint8_t columnEmpty(uint8_t *base, uint32_t stride, uint32_t height)
{
   
    for(uint32_t y=0;y<height;y++)
    {
        if(base[y*stride]) return 0;
    }
    return 1;
}

class admGlyph
{
private:
public:
    uint32_t width;
    uint32_t height;
    uint8_t  *data;
    
    

        admGlyph *next;  
        char * code;    
                admGlyph(uint32_t w,uint32_t h);
                ~admGlyph();
        uint8_t create(uint8_t *data, uint32_t stride);
};

admGlyph *searchGlyph(admGlyph *startGlyph, admGlyph *candidate);
uint8_t  destroyGlyphTree(admGlyph *startGlyph);
uint8_t  insertInGlyphTree(admGlyph *startGlyph, admGlyph *candidate);
admGlyph *clippedGlyph(admGlyph *in);
admGlyph *glyphSearchFather(admGlyph *in,admGlyph *head );

uint8_t loadGlyph(char *name,admGlyph *head,uint32_t *outNb);
uint8_t saveGlyph(char *name,admGlyph *head,uint32_t nb);


#endif
