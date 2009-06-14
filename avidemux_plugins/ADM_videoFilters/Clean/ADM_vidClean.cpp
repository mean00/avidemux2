/***************************************************************************
                          ADM_vidClean.cpp  -  description
                             -------------------
    begin                : Sun Apr 14 2002
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
#include "ADM_default.h"


#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_video/ADM_vidClean.h"



static FILTER_PARAM smoothParam={2,{"radius","blend"}};


SCRIPT_CREATE(smooth_script,AVDMVideoSmooth,smoothParam);

 char *AVDMVideoSmooth::printConf(void)
{
static char str[40];
		sprintf(str,"Smooth-Clean : R = %02lu, Blend=%01lu",_param->radius*2,_param->blend);
 		return str; // this one is pure
}
BUILD_CREATE(smooth_create,AVDMVideoSmooth);

//_______________________________________________________________

AVDMVideoSmooth::AVDMVideoSmooth(
									AVDMGenericVideoStream *in,CONFcouple *couples)
{


  	_in=in;		
   	memcpy(&_info,_in->getInfo(),sizeof(_info));  		
	if(couples)
	{
 		_param=NEW( SMOOTH_PARAMS);
		GET(radius);
		GET(blend);
	}
    else
    	{
         	_param=NEW( SMOOTH_PARAMS);
         	_param->radius=3;
              	_param->blend=1;
        }

					
  
  _uncompressed=new ADMImage (_in->getInfo()->width,_in->getInfo()->height);
  ADM_assert(_uncompressed);
  _info.encoding=1;

  	  	
}

uint8_t	AVDMVideoSmooth::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(2);

	CSET(radius);
	CSET(blend);
			return 1;

}
AVDMVideoSmooth::~AVDMVideoSmooth()
{
 	delete _uncompressed;
 	DELETE(_param);
	_uncompressed=NULL;
}

uint8_t AVDMVideoSmooth::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
uint8_t *dst,*dstu,*dstv,*src,*srcu,*srcv;

            	int16_t l,u=0,v=0;
             	int16_t nb;
              int16_t fl,fu,fv;
              int16_t	ldelta,udelta,vdelta;
              int16_t   threshold=10,su=0,sv=0;

			if(frame>=_info.nb_frames) return 0;
			ADM_assert(_uncompressed);					
								
		// read uncompressed frame
       		if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;

         	src=YPLANE(_uncompressed);
           	srcu=UPLANE(_uncompressed);;
           	srcv=VPLANE(_uncompressed);;

              dst=YPLANE(data);
              dstu=UPLANE(data);
              dstv=VPLANE(data);

              int16_t radius=_param->radius;

         		for(int32_t y=0;y<(int32_t)(_info.height );y++)
           	{
		         		for(int32_t x=0;x<(int32_t)(_info.width );x++)
             			{
                      	// for each pixel we take the surrounding one
                       	// if threshold is not met
                        		l=getPixel(x,y,_uncompressed->data);
                          	if(!(x&1))
                           	{
                          		u=getPixelU(x,y,srcu);
                          		v=getPixelU(x,y,srcv);
                            }
                            nb=0;
                            fl=0;fu=0;fv=0;


                             //------------------------                        	                        		
                   	    	for(int16_t yy=-radius+1;yy<radius;yy++)
                         	{                       			

                        	    	for(int16_t xx=-radius+1;xx<radius;xx++)
                              		{
                                  		if( (xx*xx+yy*yy)<radius*radius)
                                    	{
                                   		ldelta =getPixel(x+xx,y+yy,_uncompressed->data)-l;
  		                            		udelta=getPixelU(x+xx,y+yy,srcu)-u;
                                     		vdelta=getPixelU(x+xx,y+yy,srcv)-v;                                       	


                                         	if((udelta*udelta<threshold*threshold)&&
                                          	(vdelta*vdelta<threshold*threshold) &&
                                           	(ldelta*ldelta<threshold*threshold))
                                          		{
                                                  	nb++;
                                                   	fl=fl+ldelta+l;
                                                    fu=fu+udelta+u;
															fv=fv+vdelta+v;
                                              	}
                                        }
                                 	 }
                                  }
                                  //----------------------------------
                                  //
                                  // average value
                                  	fl=fl/nb;
                                 	fu=fu/nb;
                                  	fv=fv/nb;
                                   // now melt it
                                  	// 50/50
                                 /*  fl=(fl+l)>>1;
                                   fu=(fu+u)>>1;
                                   fv=(fv+v)>>1;*/

                                 	*dst++=fl;
                                  if(y&1)       	
                                  if(x&1)				
                                  	{
                        			setPixelU(  (su+fu)>>1,x,y,dstu);
                           			setPixelU(  (sv+fv)>>1,x,y,dstv);  
					}
                                	else
                                 	{
                                     	su=fu;
                                      	sv=fv;
                                    }

                  	}          // end for x
           		
              }     // end for y
	        data->copyInfo(_uncompressed);
              return 1;
}
uint8_t AVDMVideoSmooth::configure( AVDMGenericVideoStream *instream)
{
UNUSED_ARG(instream);

SMOOTH_PARAMS *par;
	
//     	par=_param;
     	//return((uint8_t)getSmoothParams(&par->radius,&par->blend));
	return 1;
#warning FIXME , CODE REMOVED AMD64/GCC4
}
