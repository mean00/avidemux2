/***************************************************************************
                          ADM_vidLargeMedian.cpp  -  description
                             -------------------
    begin                : Wed Jan 1 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr
    
    Using http://ndevilla.free.fr/median/median/node20.html
    optimized median search
    
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
#include "DIA_coreToolkit.h"
#include "convolution.h"
#include "convolution_desc.cpp"
#include "DIA_factory.h"
#include "ADM_largeMedian.h"
// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   largeMedian,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_NOISE,            // Category
                        "largeMedian",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("largemedian","Large Median (5x5)."),            // Display name
                        QT_TRANSLATE_NOOP("largemedian","Median filter on 5x5 matrix.") // Description
                    );


/**
    \fn ctor
*/
largeMedian::largeMedian(ADM_coreVideoFilter *previous,CONFcouple *couples)
:  ADM_coreVideoFilter(previous,couples)
{

    if(!couples || !ADM_paramLoad(couples,convolution_param,&param))
    {
        param.chroma=1;
        param.luma=1;
	}
    image=new ADMImageDefault(info.width,info.height);
    myName="largeMedian";
}

/**
    \fn dtor
*/
largeMedian::~largeMedian()
{
	if(image) delete image;
    image=NULL;
}
/**
    \fn configure
*/
bool largeMedian::configure(void)
{
  
  diaElemToggle luma(&(param.luma),QT_TRANSLATE_NOOP("largemedian","_Process luma"),QT_TRANSLATE_NOOP("largemedian","Process luma plane"));
  diaElemToggle chroma(&(param.chroma),QT_TRANSLATE_NOOP("largemedian","P_rocess chroma"));
  
  diaElem *elems[2]={&luma,&chroma};
  
  return diaFactoryRun(QT_TRANSLATE_NOOP("largemedian","Fast Convolution"),2,elems);
}
/**
    \fn getConfiguration
*/
const char   *largeMedian::getConfiguration(void)
{
    return "";
}
/**
    \fn getCoupledConf
*/
bool         largeMedian::getCoupledConf(CONFcouple **couples)
{
 return ADM_paramSave(couples, convolution_param,&param);
}

void largeMedian::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, convolution_param, &param);
}

/**
    \fn processPlane
*/
bool largeMedian::processPlane(ADMImage *s,ADMImage *d,ADM_PLANE plane)
{
        uint8_t *src=s->GetReadPtr(plane);
        uint8_t *dst=d->GetWritePtr(plane);
        uint32_t sPitch=s->GetPitch(plane);
        uint32_t dPitch=d->GetPitch(plane);
        uint32_t w=info.width;  
        uint32_t h=info.height;
        if(plane!=PLANAR_Y) 
        {
            w>>=1;
            h>>=1;
        }
	// 2xfirst and 2xlast line
		memcpy(dst,src,w);
        memcpy(dst+dPitch,src+sPitch,w);
        memcpy(dst+(h-1)*dPitch,src+(h-1)*sPitch,w);
        memcpy(dst+(h-2)*dPitch,src+(h-2)*sPitch,w);
    // Other lines
        uint8_t *o,*p1,*p2,*c,*n1,*n2;
        o=dst+dPitch+dPitch;
		p1=src;
		p2=p1+sPitch;
		c=p2+sPitch;
        n1=c+sPitch;
        n2=n1+sPitch;

		// Luma
		for(int32_t y=2;y<h-2;y++)
		{
			doLine(p1,p2,c,n1,n2,o,w);
			p1=p2;
            p2=c;
            c=n1;
            n1=n2;
            n2+=sPitch;
			o+=dPitch;
		}
        return true;
}
/**
    \fn getNextFrame
*/
bool largeMedian::getNextFrame(uint32_t *fn,ADMImage *data)
{
//uint8_t *dst,*dstu,*dstv,*srcu,*srcv;
uint8_t *x1,*x2,*x3,*x4,*x5,*o1;
uint32_t stride,page;
    ADM_assert(image);					
    if(!previousFilter->getNextFrame(fn,image)) return 0;             
	
																
 // Luma...
	if(!param.luma)
	{
        ADMImage::copyPlane(image,data,PLANAR_Y);
	}
	else
	{
        processPlane(image,data,PLANAR_Y);
	}
    // chroma u & v
	if(!param.chroma)
	{
        ADMImage::copyPlane(image,data,PLANAR_U);
        ADMImage::copyPlane(image,data,PLANAR_V);
	}
	else
	{
        processPlane(image,data,PLANAR_U);
        processPlane(image,data,PLANAR_V);
	}
	data->copyInfo(image);
    return true;
}
         

/**
    \fn doLine
*/
uint8_t largeMedian::doLine(uint8_t  *pred2,uint8_t  *pred1,
					uint8_t *cur,
   					uint8_t *next1,uint8_t *next2,
   					uint8_t *out,
                       			uint32_t w)
                                 
{
static uint8_t box[5][5];	
static uint8_t box2[5][5];	

uint32_t col;
uint8_t temp;
uint32_t inbox;
	
// prefill box
	for(uint32_t x=0;x<4;x++)
		{
			box[0][x+1]=*(pred2+x);
			box[1][x+1]=*(pred1+x);
			box[2][x+1]=*(cur+x);
			box[3][x+1]=*(next1+x);
			box[4][x+1]=*(next2+x);			
		}
		col=0;
		*out=*cur;
		*(out+1)=*(cur+1);
		*(out+w-1)=*(cur+w-1);		
		*(out+w-2)=*(cur+w-2);
		out+=2;
		next1+=4;
		next2+=4;
		pred1+=4;
		pred2+=4;
		cur+=4;	
	while(w>4)
	{
		// fill
			box[0][col]=*pred2++;
			box[1][col]=*pred1++;
			box[2][col]=*cur++;
			box[3][col]=*next1++;
			box[4][col]=*next2++;
			col++;
			col%=5;
			// copy & sort
			memcpy(box2,box,5*5);
			uint8_t *p=(uint8_t *)box2;	
			inbox=0;	
#define PIX_SORT(a,b) { if ((a)>(b)) PIX_SWAP((a),(b)); }
#define PIX_SWAP(a,b) { temp=(a);(a)=(b);(b)=temp; }

			
    PIX_SORT(p[0], p[1]) ;   PIX_SORT(p[3], p[4]) ;   PIX_SORT(p[2], p[4]) ;
    PIX_SORT(p[2], p[3]) ;   PIX_SORT(p[6], p[7]) ;   PIX_SORT(p[5], p[7]) ;
    PIX_SORT(p[5], p[6]) ;   PIX_SORT(p[9], p[10]) ;  PIX_SORT(p[8], p[10]) ;
    PIX_SORT(p[8], p[9]) ;   PIX_SORT(p[12], p[13]) ; PIX_SORT(p[11], p[13]) ;
    PIX_SORT(p[11], p[12]) ; PIX_SORT(p[15], p[16]) ; PIX_SORT(p[14], p[16]) ;
    PIX_SORT(p[14], p[15]) ; PIX_SORT(p[18], p[19]) ; PIX_SORT(p[17], p[19]) ;
    PIX_SORT(p[17], p[18]) ; PIX_SORT(p[21], p[22]) ; PIX_SORT(p[20], p[22]) ;
    PIX_SORT(p[20], p[21]) ; PIX_SORT(p[23], p[24]) ; PIX_SORT(p[2], p[5]) ;
    PIX_SORT(p[3], p[6]) ;   PIX_SORT(p[0], p[6]) ;   PIX_SORT(p[0], p[3]) ;
    PIX_SORT(p[4], p[7]) ;   PIX_SORT(p[1], p[7]) ;   PIX_SORT(p[1], p[4]) ;
    PIX_SORT(p[11], p[14]) ; PIX_SORT(p[8], p[14]) ;  PIX_SORT(p[8], p[11]) ;
    PIX_SORT(p[12], p[15]) ; PIX_SORT(p[9], p[15]) ;  PIX_SORT(p[9], p[12]) ;
    PIX_SORT(p[13], p[16]) ; PIX_SORT(p[10], p[16]) ; PIX_SORT(p[10], p[13]) ;
    PIX_SORT(p[20], p[23]) ; PIX_SORT(p[17], p[23]) ; PIX_SORT(p[17], p[20]) ;
    PIX_SORT(p[21], p[24]) ; PIX_SORT(p[18], p[24]) ; PIX_SORT(p[18], p[21]) ;
    PIX_SORT(p[19], p[22]) ; PIX_SORT(p[8], p[17]) ;  PIX_SORT(p[9], p[18]) ;
    PIX_SORT(p[0], p[18]) ;  PIX_SORT(p[0], p[9]) ;   PIX_SORT(p[10], p[19]) ;
    PIX_SORT(p[1], p[19]) ;  PIX_SORT(p[1], p[10]) ;  PIX_SORT(p[11], p[20]) ;
    PIX_SORT(p[2], p[20]) ;  PIX_SORT(p[2], p[11]) ;  PIX_SORT(p[12], p[21]) ;
    PIX_SORT(p[3], p[21]) ;  PIX_SORT(p[3], p[12]) ;  PIX_SORT(p[13], p[22]) ;
    PIX_SORT(p[4], p[22]) ;  PIX_SORT(p[4], p[13]) ;  PIX_SORT(p[14], p[23]) ;
    PIX_SORT(p[5], p[23]) ;  PIX_SORT(p[5], p[14]) ;  PIX_SORT(p[15], p[24]) ;
    PIX_SORT(p[6], p[24]) ;  PIX_SORT(p[6], p[15]) ;  PIX_SORT(p[7], p[16]) ;
    PIX_SORT(p[7], p[19]) ;  PIX_SORT(p[13], p[21]) ; PIX_SORT(p[15], p[23]) ;
    PIX_SORT(p[7], p[13]) ;  PIX_SORT(p[7], p[15]) ;  PIX_SORT(p[1], p[9]) ;
    PIX_SORT(p[3], p[11]) ;  PIX_SORT(p[5], p[17]) ;  PIX_SORT(p[11], p[17]) ;
    PIX_SORT(p[9], p[17]) ;  PIX_SORT(p[4], p[10]) ;  PIX_SORT(p[6], p[12]) ;
    PIX_SORT(p[7], p[14]) ;  PIX_SORT(p[4], p[6]) ;   PIX_SORT(p[4], p[7]) ;
    PIX_SORT(p[12], p[14]) ; PIX_SORT(p[10], p[14]) ; PIX_SORT(p[6], p[7]) ;
    PIX_SORT(p[10], p[12]) ; PIX_SORT(p[6], p[10]) ;  PIX_SORT(p[6], p[17]) ;
    PIX_SORT(p[12], p[17]) ; PIX_SORT(p[7], p[17]) ;  PIX_SORT(p[7], p[10]) ;
    PIX_SORT(p[12], p[18]) ; PIX_SORT(p[7], p[12]) ;  PIX_SORT(p[10], p[18]) ;
    PIX_SORT(p[12], p[20]) ; PIX_SORT(p[10], p[20]) ; PIX_SORT(p[10], p[12]) ;
			
		  
		  *out++=p[12];
		  w--;
	}	
	
	return 1;
}
// EOF

