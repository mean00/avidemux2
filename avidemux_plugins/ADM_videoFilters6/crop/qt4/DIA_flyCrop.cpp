/***************************************************************************
                          DIA_flyCrop.cpp  -  description
                             -------------------

        Common part of the crop dialog
    
    copyright            : (C) 2002/2007 by mean
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
#include "DIA_flyDialogQt4.h"
#include "ADM_default.h"
#include "ADM_image.h"
#include "DIA_flyCrop.h"


static uint8_t 		Metrics( uint8_t *in, uint32_t width,uint32_t *avg, uint32_t *eqt);
static uint8_t 		MetricsV( uint8_t *in, uint32_t width,uint32_t height,uint32_t *avg, uint32_t *eqt);

/**
 * \fn blank
 * \brief Green bars
 */
static void blank(uint8_t *in, int w, int h, int stride)
{
    for(int y=0;y<h;y++)
    {
        memset(in,0,4*w);
        uint8_t *green=in+1;
        for(int x=0;x<w;x++)
            green[x<<2]=0xff;
        uint8_t *alpha=in+3;
        for(int x=0;x<w;x++)
            alpha[x<<2]=0xff;
        in+=stride;
    }
}

/**
 * \fn processRgb
 * @param imageIn
 * @param imageOut
 * @return
 */
uint8_t flyCrop::processRgb(uint8_t *imageIn, uint8_t *imageOut)
{
    int stride=ADM_IMAGE_ALIGN(_w*4);
    memcpy(imageOut,imageIn,stride*_h);

    blank(imageOut,_w,top,stride);
    blank(imageOut+stride*(_h-bottom),_w,bottom,stride);
    blank(imageOut,left,_h,stride);
    blank(imageOut+(_w-right)*4,right,_h,stride);
    return true;
}

/**
     \fn autocrop
	\brief 
*/

uint8_t  flyCrop::autocrop(void)
{
uint8_t *in;
uint32_t y,avg,eqt;
	// Top

#define THRESH_AVG   30
#define THRESH_EQT   50
        
        in=_yuvBuffer->GetReadPtr(PLANAR_Y);
        for(y=0;y<((_h>>1)-2);y++)	
        {
                Metrics(in,_w,&avg,&eqt);
                in+=_w;
                //printf("LineT :%d avg: %d eqt: %d\n",y,avg,eqt);
                if(avg> THRESH_AVG || eqt > THRESH_EQT)
                        break;
        }
//gotcha_:	
        if(y)
                top=y-1;
        else 
                top=0;
                
        in=_yuvBuffer->GetReadPtr(PLANAR_Y)+_w*(_h-1);
        for(y=0;y<((_h>>1)-2);y++)	
        {
                Metrics(in,_w,&avg,&eqt);
                in-=_w;
                //printf("Line B :%d avg: %d eqt: %d\n",y,avg,eqt);
                if(avg> THRESH_AVG || eqt > THRESH_EQT)
                                break;
        }
//gotcha_:	
        if(y)
                bottom=y-1;
        else
                bottom=0;

                
// Left
        in=_yuvBuffer->GetReadPtr(PLANAR_Y);
        for(y=0;y<((_w>>1)-2);y++)	
        {
                MetricsV(in,_w,_h,&avg,&eqt);
                in++;
                //printf("Line L :%d avg: %d eqt: %d\n",y,avg,eqt);
                if(avg> THRESH_AVG || eqt > THRESH_EQT)
                                break;
        }
//gotcha_:	
        if(y)
                left=y-1;
        else
                left=0;		
// Right
        in=_yuvBuffer->GetReadPtr(PLANAR_Y)+_w-1;
        for(y=0;y<((_w>>1)-2);y++)	
        {
                MetricsV(in,_w,_h,&avg,&eqt);
                in--;
                //printf("Line R :%d avg: %d eqt: %d\n",y,avg,eqt);
                if(avg> THRESH_AVG || eqt > THRESH_EQT)
                                break;
        }
//gotcha_:	
        if(y)
                right=y-1;
        else
                right=0;
  
              
        // Update display
        top=top & 0xfffe;
        bottom=bottom & 0xfffe;
        upload();
        sameImage();
        return 1;
}
/**
     \fn Metrics
	\brief Compute the average value of pixels	and eqt is the "ecart type"
*/

uint8_t Metrics( uint8_t *in, uint32_t width,uint32_t *avg, uint32_t *eqt)
{

uint32_t x;
uint32_t sum=0,eq=0;
uint8_t v;
              for(x=0;x<width;x++)
              {
                      sum+=*(in+x);
              }
              sum=sum/width;
              *avg=sum;
              for(x=0;x<width;x++)
              {
                      v=*(in+x)-sum;
                      eq+=v*v;
              }
              eq=eq/(width*width);
              *eqt=eq;
              return 1;
}
/**
     \fn MetricsV
	\brief Compute the average value of pixels	and eqt is the "ecart type"
*/
uint8_t MetricsV( uint8_t *in,uint32_t width, uint32_t height,uint32_t *avg, uint32_t *eqt)
{

uint32_t x;
uint32_t sum=0,eq=0;
uint8_t v;
              for(x=0;x<height;x++)
              {
                      sum+=*(in+x*width);
              }
              sum=sum/height;
              *avg=sum;
              for(x=0;x<height;x++)
              {
                      v=*(in+x*width)-sum;
                      eq+=v*v;
              }
              eq=eq/(height*height);
              *eqt=eq;
              return 1;
}
//EOF

