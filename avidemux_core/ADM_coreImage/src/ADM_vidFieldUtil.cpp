/***************************************************************************
                          ADM_vidFieldUtil.h  -  description
                             -------------------

Some functions to manipulate fields

    begin                : Sun Apr 11 2003
    copyright            : (C) 2003 by mean
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


//#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_vidFieldUtil.h"


void vidFieldDecimate(uint8_t *src,uint8_t *target, uint32_t linessrc, uint32_t width)
{
	linessrc>>=1;
	for(;linessrc>0;linessrc--)
		{
			memcpy(target,src,width);
			target+=width;
			src+=width*2;
		}


}

void vidFieldKeepOdd(uint32_t w,uint32_t h,uint8_t *src,uint8_t *target)
{

		uint32_t page=w*h;

 		vidFieldDecimate(src+w,target,h*2,w);

		src+=page*2;
		target+=page;
		vidFieldDecimate(src+(w>>1),target,h,w>>1);

		src+=page>>1;
		target+=page>>2;
		vidFieldDecimate(src+(w>>1),target,h,w>>1);
}
void vidFieldKeepEven(uint32_t w,uint32_t h,uint8_t *src,uint8_t *target)
{
	uint32_t page=w*h;

		vidFieldDecimate(src,target,h*2,w);

		src+=page*2;
		target+=page;
		vidFieldDecimate(src,target,h,w>>1);

		src+=page>>1;
		target+=page>>2;
		vidFieldDecimate(src,target,h,w>>1);
}
//
//
//
static void stack(uint32_t w,uint32_t h,uint8_t *src,uint8_t *target)
{
uint8_t *dst1,*dst2;
	dst1=target;
	dst2=target+((w*h)>>1);	
	
	for(uint32_t y=h>>1;y>0;y--)
	{
		memcpy(dst1,src,w);
		memcpy(dst2,src+w,w);
		src+=2*w;
		dst1+=w;
		dst2+=w;
	}
	
}
//
//
//
static void unstack(uint32_t w,uint32_t h,uint8_t *src,uint8_t *target)
{
uint8_t *dst,*src1,*src2;
        src1=src;
        src2=src+((w*h)>>1); 
        dst=target;
        for(uint32_t y=h>>1;y>0;y--)
        {
                memcpy(dst,src1,w);
                memcpy(dst+w,src2,w);
                dst+=2*w;
                src1+=w;
                src2+=w;
        }
        
}

//
//	Put field 1 on top of field2
//
uint8_t vidFielStack(uint32_t w,uint32_t h,uint8_t *src,uint8_t *target)
{
uint8_t *d,*s;
//uint32_t y;

// interleave Y
uint32_t page=w*h;
	d=target;
	s=src;
	
	stack(w,h,s,d);
	
	d=target+page;
	s=src+page;
	stack(w>>1,h>>1,s,d);
	
	d=target+((page*5)>>2);
	s=src+((page*5)>>2);
	stack(w>>1,h>>1,s,d);
	
	return 1;
}
//
//      Put field 1 on top of field2
//
uint8_t vidFielUnStack(uint32_t w,uint32_t h,uint8_t *src,uint8_t *target)
{
uint8_t *d,*s;
//uint32_t y;

// interleave Y
uint32_t page=w*h;
        d=target;
        s=src;
        
        unstack(w,h,s,d);
        
        d=target+page;
        s=src+page;
        unstack(w>>1,h>>1,s,d);
        
        d=target+((page*5)>>2);
        s=src+((page*5)>>2);
        unstack(w>>1,h>>1,s,d);
        
        return 1;
}
	

void vidFieldMerge(uint32_t w,uint32_t h,uint8_t *src,uint8_t *src2,uint8_t *target)
{
uint8_t *d,*s,*s2;
//uint32_t y;

// interleave Y

	d=target;
	s=src;
	s2=src2;
	for(uint32_t y=h>>1;y>0;y--)
	{
		memcpy(d,s,w);
		memcpy(d+w,s2,w);
		s+=w;
		s2+=w;
		d+=w*2;
	}
	//
	d=target+(w*h);
	s=src+((w*h)>>1);
	s2=src2+((w*h)>>1);
	for(uint32_t y=h>>1;y>0;y--)
	{
		memcpy(d,s,w>>1);
		memcpy(d+(w>>1),s2,w>>1);
		s+=w>>1;
		s2+=w>>1;
		d+=w;
	}

}

