//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ADM_assert.h"


#include "fourcc.h"

#include "config.h"
#include "avi_vars.h"


#include "ADM_leftturn.h"

static int incx[4]={1,0,-1,0};
static int incy[4]={0,1,0,-1};

static int nextdir[4]={1,0,3,2};


uint8_t adm_estimate_glyphSize(admGlyph *glyph,uint32_t *minx, uint32_t *maxx,uint32_t *miny,uint32_t *maxy,int *raw)
{
    uint32_t nbPoints=0;
    
    int      startx,starty;
    int      curx,cury;
    int      tgtx,tgty;
    int      dir;
    int      found;
    uint32_t xmin=glyph->width-1;
    uint32_t xmax=0;
    
    uint32_t ymin=glyph->height-1,ymax=0;
    
    for(uint32_t i=0;i<glyph->height;i++) raw[i]=-1;
    
    
    *minx=0;
    *maxx=0;
    // Small glyph ?
    if(glyph->width<3)
    {
        *minx=0;
        *maxx=glyph->width-1;
        return 1;
    }
    // Search for a non empty line from the bottom of the glyph bottom to top, left to right
    startx=0;
    for(starty=glyph->height-1;starty>=0;starty--)
    {
       
            if(glyph->data[starty*glyph->width]) goto _fnd;   
        
    }
    // Glyph empty ?
    printf("Empty glyph\n");
    return 0;
_fnd:
    dir=0;
    curx=startx;
    cury=starty;
    //printf("Starting :%d %d (%d x %d )\n",startx,starty,glyph->width,glyph->height);
    
    while(1)
    {   
        found=0;
        for(uint32_t i=0;i<4;i++)
        {   
            tgtx=curx+incx[(dir+nextdir[i])%4];
            tgty=cury+incy[(dir+nextdir[i])%4];
            //printf("\t candidate :%d %d dir %d\n",tgtx,tgty,(dir+i)%4);
            if(tgtx>=0 && tgty>=0 && tgtx<glyph->width && tgty<glyph->height)   // Still into the glyph bounding box ?
                if(glyph->data[tgtx+glyph->width*tgty])                                // Still inside glyph ?
                {
                    dir=(dir+nextdir[i])%4;
                    found=1;
                    curx=tgtx;
                    cury=tgty;
                    break;
                    
                }
        }     
        if(!found)
        {          
            printf("!Stuck\n");
            return 0;
        }
        if(curx>xmax) xmax=curx;
        if(curx<xmin) xmin=curx;
        if(cury>ymax) ymax=cury;
        if(cury<ymin) ymin=cury;
        if((int)curx>raw[cury])
        {
            raw[cury]=curx;
        }
        
       // printf("New coord:%d %d, dir=%d\n",curx,cury,dir);
        
        if(curx==startx && cury==starty) 
        {
            *minx=xmin;
            *maxx=xmax;
    
            *miny=ymin;
            *maxy=ymax;
            return 1;        
        }        
    }   
    return 0;
}
