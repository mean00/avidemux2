/***************************************************************************
                          ADM_resizebis.cpp  -  description
                             -------------------

      Strongly derivated from avisynth

      Seems that int on VC++ is a long integer, hence the INT cast.


    begin                : Sun Mar 24 2002
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

#include "ADM_videoFilterDynamic.h"
#include "ADM_vidResize.h"
#include "ADM_resizebis.hxx"

#define SLOW

// General purpose, no need to put them inside class
// declare static as it could lead to some optimizations
// the original file was plain C

double TriangleFunc(double x);
double MitchellFunc(double x);
double Lanczos3f(double x);
double Lanczos3sinc(double x);


ResampleFunc TriangleFilter = {
    TriangleFunc, 1.0
};

ResampleFunc MitchellFilter = {
    MitchellFunc, 2.0
};

ResampleFunc LanczosFilter = {
    Lanczos3f, 3.0
};



ResampleFunc RFuncs[3] = {
    TriangleFilter, MitchellFilter,LanczosFilter
};


//________________________________________________

double TriangleFunc(double x)
{
    x = fabs(x);
    return (x < 1.0) ? 1.0 - x : 0.0;
}


#define bbb (1./3.)
#define ccc (1./3.)
#define p0  ((6. - 2. * bbb) / 6.)
#define p2  ((-18. + 12. * bbb + 6. * ccc) / 6.)
#define p3  ((12. - 9. * bbb - 6. * ccc) / 6. )
#define q0  ((8. * bbb + 24. * ccc) / 6.   )
#define q1  ((-12. * bbb - 48. * ccc) / 6.)
#define q2  ((6. * bbb + 30. * ccc) / 6.)
#define   q3 ( (     -     bbb -  6.*ccc) / 6.)
double MitchellFunc(double x)
{
    x = fabs(x);
    return (x < 1.0) ? (p0 + x * x * (p2 + x * p3)) :
      (x <  2.) ? (q0 +     x * (q1 +  x *  (q2   +   x   *   q3))): 0.0;
}
double Lanczos3sinc(double value)
{
	if (value != 0.0)
	{
		value *= M_PI;
		return sin(value) / value;
	}
	else
	{
		return 1.0;
	}
}

double Lanczos3f(double value)
{
	if (value < 0.0)
	{
		value = -value;
	}
	if (value < 3.0)
	{
		return (Lanczos3sinc(value) * Lanczos3sinc(value / 3.0));
	}
	else
	{
		return 0.0;
	}
}



// This function returns a resampling "program" which is interpreted by the
// FilteredResize filters.  It handles edge conditions so FilteredResize
// doesn't have to.
//____________________________________________________________________________

INT *GetResamplingPattern(uint32_t original_width,
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
    int fir_filter_size = int (ceil(filter_support * 2));
    INT *result = new INT[1 + target_width * (1 + fir_filter_size)];
    INT *cur = result;
    
    *cur++ = fir_filter_size;
   
    printf("\n Fir size : %d",fir_filter_size);
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
		    *cur++ =int (total3 * 65536 + 0.5) - int (total2 * 65536 +
						      0.5);
		total2 = total3;
	    } 
            pos += pos_step;
      } 
      return result;
}

//
//      Vertical resizing
//              Resized one plane also
//_______________________________
void AVDMVideoStreamResize::ResizeV(Image * iin, Image * iout,
				    INT *pattern)
{
    INT *cur = pattern;
    int fir_filter_width = *cur++;
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
					 total = 0;
					 base2 = base + x;
					for (int b = 0; b < fir_filter_width; ++b)
				  	{	
				      		total += *base2 * cur[b];
		      				base2 += row_size;
					  }
					  *out++ = ScaledPixelClip(total);
	  			  } 
				cur += fir_filter_width;
	}
}

//____________________________________
//      Perform horizontal resizing
//              On one plane
//              Must be called 3 times for RGB or 3 times for YV12
//____________________________________
void AVDMVideoStreamResize::ResizeH(Image * iin, Image * iout,
				    INT  *pattern)
{
    unsigned char *base = iin->data;
    unsigned char *out = iout->data;
    int src_row_size = iin->width;
    int dst_row_size = iout->width;
	uint8_t *ptr;
	INT *cur;
	int32_t total;
   INT ofs;
    
    for (uint32_t y =  iin->height; y>0; y--)
      {
	  	cur = pattern + 1;
	  for (int x = 0; x < dst_row_size ; x++)
	    {
			total=0;
			ofs = *cur++;
			ptr= &(base[(ofs )]);

			for (int a =   pattern[0];a>0; a--)
			  {
			      total += (*ptr++) * (*cur++);
		  		}
		   		out[x] = ScaledPixelClip(total);
	    } // next line...
	     base += src_row_size;
	  	out += dst_row_size;
	}

}

///_____________

/*  ____________________________________________________________________
	zoom()

	Resizes bitmaps while resampling them.
	Returns -1 if error, 0 if success.
*/
void AVDMVideoStreamResize::precompute(Image * dst, Image * src,
				       uint8_t algo)
//____________________________________________________________________
{

#define FREE_USED(x) if(x) { delete [] x;x=NULL;}
	FREE_USED(Hpattern_luma);
	FREE_USED(Hpattern_chroma);
	FREE_USED(Vpattern_luma);
	FREE_USED(Vpattern_chroma);
	
#ifdef SLOW	 
	#define MakePattern GetResamplingPattern
#else
	#define MakePattern GetResamplingPatternFIR4
#endif

    Hpattern_luma = MakePattern(src->width,  dst->width, &RFuncs[algo]);
    Hpattern_chroma =MakePattern(src->width >> 1,  dst->width >> 1, &RFuncs[algo]);
    Vpattern_luma =MakePattern(src->height,  dst->height,&RFuncs[algo]);
    Vpattern_chroma =MakePattern(src->height >> 1,   dst->height >> 1, &RFuncs[algo]);


}

//__________________________________________________________________
uint8_t AVDMVideoStreamResize::zoom(Image * dst, Image * src)				
//__________________________________________________________________
{
  // First Horizontal resize from src -> intermediate
  Image tmpin;
                      tmpin.width=dst->width;
                      tmpin.height=src->height;
                      tmpin.data=_intermediate_buffer;
		      

#ifdef SLOW		 											
					#define DORES(src,dst,patH,patV)   	ResizeH(src, &tmpin,patH)    ;  \
													        		        ResizeV(&tmpin,dst,patV);        
#else                     
					#define DORES(src,dst,patH,patV)    ResizeHFIR4(src, &tmpin,patH)    ; \
													           					ResizeVFIR4(&tmpin,dst,patV);      
#endif
		      
		      
		      //---------Y---------------
		      
		      	DORES(src,dst,Hpattern_luma, Vpattern_luma);
    				

				// update fields, 
    				src->data+=src->width*src->height;
    				dst->data+=dst->width*dst->height;
    				
    		    src->width>>=1;
    		    src->height>>=1;
    		    dst->width>>=1;
    		    dst->height>>=1;
          
		       	tmpin.height=src->height;
		       	tmpin.width=dst->width;
		       	tmpin.data+= tmpin.height*tmpin.width;

		       // --------- U -------------
		      	DORES(src,dst,Hpattern_chroma, Vpattern_chroma);
		          				
    		    src->data+=src->width*src->height;
    				dst->data+=dst->width*dst->height;
    				tmpin.data+= tmpin.height*tmpin.width;
		       // --------- V -------------
		      
		      	DORES(src,dst,Hpattern_chroma, Vpattern_chroma);    				
    		       
    return 1;
}				/* zoom */


// clean up insiders datas
//
void AVDMVideoStreamResize::endcompute(void)
{
	FREE_USED(Hpattern_luma);
	FREE_USED(Hpattern_chroma);
	FREE_USED(Vpattern_luma);
	FREE_USED(Vpattern_chroma);
}



