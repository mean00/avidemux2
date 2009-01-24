/***************************************************************************
                          ADM_resizeter.cpp  -  description
                             -------------------
                             
		This is an optimised with FIR size=4
		of vidResizebis
		
    Faster but lesser quality		
    (nb sample to compute is potentially smaller)                                                          
                             
    begin                : Sat Sep 21 2002
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
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidResize.h"
#include "ADM_resizebis.hxx"
 
 
 /*
  	Same as above but use a fixed size FIR size of 4
   	Good until ~ half width


*/
INT *GetResamplingPatternFIR4(uint32_t original_width,
			      uint32_t target_width, ResampleFunc * func)
{
    double scale, filter_step,filter_support;
    double pos_step ;

    scale = double (target_width);
    scale/= (double)original_width;
    if (scale < 1.0)
	filter_step = scale;

    else
	filter_step = 1.0;
    filter_support = func->support;
    filter_support /= filter_step;
    
    int fir_filter_size = 3;
    
    
    int16_t  *result = new int16_t[1 + target_width * (1 + fir_filter_size)];
    int16_t  *cur = result;
    
    *cur++ = fir_filter_size;
   
    printf("\n Fixed Fir size : %d",fir_filter_size);
    pos_step=original_width;
    pos_step /= target_width;

 

    // the following translates such that the image center remains fixed
    double pos = original_width;

       pos-= target_width ;
       pos/= (target_width * 2);

//   printf("\n In width : %lu out width : %lu pos_step : %lf pos_initial : %lf",original_width,target_width, pos_step,pos);

    for (uint32_t i = 0; i < target_width; ++i)

      {
	  INT end_pos = (INT) (pos + filter_support);
	  if (end_pos > (INT)original_width - 1)
	      end_pos = original_width - 1;
	  INT start_pos = end_pos - fir_filter_size + 1;
	  if (start_pos < 0)
	      start_pos = 0;
	  *cur++ = start_pos;

	  // the following code ensures that the coefficients add to exactly 65536
	  double total = 0.0;
	  for (int j = 0; j < fir_filter_size; ++j)
	      total += func->f((start_pos + j - pos) * filter_step);
	  double total2 = 0.0;
	  for (int k = 0; k < fir_filter_size; ++k)

	    {
		double total3 =
		    total2 +func->f((start_pos + k - pos) * filter_step) / total;
		    *cur++ =int (total3 * 256 + 0.5) - int (total2 * 256 +
						      0.5);
		total2 = total3;
	    } 
            pos += pos_step;
      } 
      return (INT *)result;
}




/*
      Vertical resizing

  		Fast resample, FIR= 4 hardcoded
    
    	the input is byte
     	the coef are signed 16 bits integere

*/
void AVDMVideoStreamResize::ResizeVFIR4(Image * iin, Image * iout,
				    INT *intpattern)
{
    int16_t *pattern = (int16_t *)intpattern;

    int16_t *cur = pattern;
    /*int fir_filter_width = *cur++*/cur++;
    int row_size = iin->width;
    unsigned char *out = iout->data;
    unsigned char *srcbuffer = iin->data;
    unsigned char *base ,*base2;
		int total;
		
			

    for (uint32_t y = 0; y < iout->height; ++y)

      {
	  			base = srcbuffer + row_size * (*cur++);
	  			for (int x = 0; x < row_size; ++x)
				  {
					
					 base2 = base + x;
					 
					    		total = *base2 							 * cur[0];
					    		total += *(base2+row_size) 		 * cur[1];
					    		total += *(base2+(row_size<<1)) * cur[2];		      			
					    		//total += *(base2 + row_size*3) * cur[3];
								  *out++ = ScaledPixelClip8(total);
	  			  } 
				cur += 3;
		}
}


/*
  		Fast resample, FIR= 4 hardcoded
    
    	the input is byte
     	the coef are signed 16 bits integere

*/
void AVDMVideoStreamResize::ResizeHFIR4(Image * iin, Image * iout,
				    INT  *bigpattern)
{
    uint8_t *base = iin->data;
    uint8_t *out 	= iout->data;
  
    int src_row_size = iin->width;
    int dst_row_size = iout->width;
    int16_t *pattern=(int16_t *)bigpattern;
    
		uint8_t 				*ptr;
		int16_t 				*cur;
		static int32_t 	total;
		//int32_t					*t=&total;
  	int16_t 				ofs;
   
	for (uint32_t y =  iin->height; y>0; y--)
      {
	  	cur = pattern + 1;	   
		  for (int x = 0; x < dst_row_size ; x++)
	    {
				ofs = *cur++;
				ptr= &(base[(ofs )]);
      	total=0;
	
				total =  *(ptr) 	* *(cur);
				total += *(ptr+1) * *(cur+1);
		  	total += *(ptr+2) * *(cur+2);
//		  	total += *(ptr+3) * *(cur+3);   		   		
      	cur+=3;
		  	*out++ = ScaledPixelClip8(total);
	    } // next line...
	     base += src_row_size;
	}
}
