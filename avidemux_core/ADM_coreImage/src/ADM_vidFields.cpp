/***************************************************************************
                          ADM_vidFields.cpp  -  description
                             -------------------
    begin                : Sun Jan 12 2003
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
#include "ADM_videoFilter.h"
#include "ADM_vidField.h"
#include "DIA_enter.h"
//_______________________________________________________________

ADMVideoFields::ADMVideoFields(
									AVDMGenericVideoStream *in,CONFcouple *couples)
{


  	_in=in;		
   	memcpy(&_info,_in->getInfo(),sizeof(_info));  		
	
					
	_motionmask=new uint8_t [_in->getInfo()->width*_in->getInfo()->height];
	ADM_assert(_motionmask);
	_motionmask2=new uint8_t [_in->getInfo()->width*_in->getInfo()->height];
	ADM_assert(_motionmask2);

	_info.encoding=1;
	if(couples)
	{
			_param=NEW(DEINT_PARAM);
//			GET(motion_trigger);
			//GET(blend_trigger);
	}
	else
	{
			_param=new( DEINT_PARAM);
			_param->blend_trigger=9;
			_param->motion_trigger=15;
	}
}

uint8_t	ADMVideoFields::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(2);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
//	CSET(motion_trigger);
//	CSET(blend_trigger);
			return 1;

}

ADMVideoFields::~ADMVideoFields()
{
 	
	delete []_motionmask ;
	delete []_motionmask2;
 	DELETE(_param);
	
}
//
//	Return 1 if seen as interleaved
//		0 is seen as progressiv
//
//		Check if in a 8x8 square n, n+1 , n+2 lines differ too much
//
uint8_t ADMVideoFields::hasMotion(ADMImage *image)
{
    	uint32_t w,h,x,y;
      	uint8_t *n,*p,*c,*e,*e2;
	uint8_t *yplane=YPLANE(image);
       

     	w=_info.width;
     	h=_info.height;



      	memset(_motionmask,0,w*h);
      	memset(_motionmask2,0,w*h);

       // First line
       	memset(_motionmask,0xff,w);
          	memset(_motionmask2,0xff,w);

        	p=yplane;
         	c=p+w;
          	n=c+w;
           e=_motionmask+w; 	
           e2=_motionmask2+w; 	
  //___________________ C version of motion detection ________________________
       // other line
#if defined(ADM_CPU_X86) && defined(ASM_DEINT)
       if(CpuCaps::hasMMX())  
      	hasMotion_MMX(p,c,n,e,e2);
       else
#endif 
      	 hasMotion_C(p,c,n,e,e2);
       
      
      
//_______________________________
           // last line
           memset(e,0xff,w);
           memset(e2,0xff,w);

           // Count    how many tagged as !=
           p=_motionmask;
           c=p+w;;
           n=c+w;;

           // 8x8 square
           uint8_t *box=new uint8_t[ ((h+8)>>3)*((w+8)>>3)]; // ???
           uint32_t boxx,boxy;

           memset(box,0,  ((h+8)>>3)*((w+8)>>3));
           for(y=h-2;y>0;y--)
           	{
                     boxy=(y>>3)*(w>>3);
                     for(x=w;x>0;x--)
                     	{
                             boxx=boxy+(x>>3);
                             if( *c&&*p&&*n)
                             	{
                                 	box[boxx]++;
                                }
                                c++;n++;p++;
                        }
              }

              // reached level ?
              for(x=   ((h+8)>>3)*((w+8)>>3);x>0;x--)
              {
                     	if (box[x]>15)
                      	{
                          	
                            	delete [] box;
                             	return 1;
                         }


                }
                       	delete [] box;
                        	return 0;

}

uint8_t ADMVideoFields::doBlend(ADMImage *src,ADMImage *dst)
{
   	uint32_t w,h,x; //,y;
      	uint8_t *n,*p,*c,*e2;
	uint8_t *f;
	uint8_t *yplane;


	
     	w=_info.width;
     	h=_info.height;

	f=YPLANE(dst);
	yplane=YPLANE(src);
	p=yplane;	
	c=yplane;
	n=c+w;
	e2=_motionmask2+w; 
	
	// First line
	// always blend
	for(x=w;x>0;x--)
	{
		*f++=(*c+*n)>>1;
		n++;
		c++;
	}
#if defined(ADM_CPU_X86) && defined(ASM_BLEND)
       if(CpuCaps::hasMMX())               
              blend_MMX(p,c,n,e2,f);
        else
#endif
              blend_C(p,c,n,e2,f);
              // Last line
            for(x=w;x>0;x--)
            {
               	*f++=(*c+*p)>>1;
                	  p++;
                   c++;

              }

              return 1;



}
uint8_t ADMVideoFields::configure( AVDMGenericVideoStream *instream)
{
int i,j;
	_in=instream;
	i=(int)_param->motion_trigger;
	j=(int)_param->blend_trigger;
	if(DIA_GetIntegerValue(&i,0,255, ("Motion Threshold"),""))
	{
		if(DIA_GetIntegerValue(&j,0,255, ("Blend Threshold"),""))
		{
			_param->motion_trigger=(uint8_t)i;
			_param->blend_trigger=(uint8_t)j;
			return 1;
		}
	} 

	return 0;    
}      



