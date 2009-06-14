/***************************************************************************
                          ADM_vidFastConvolution.cpp  -  description
                             -------------------
    begin                : Sat Nov 23 2002
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
#include "DIA_factory.h"
#include "ADM_vidConvolution.hxx"



static FILTER_PARAM convParam={2,{"luma","chroma"}};
VF_DEFINE_FILTER(AVDMFastVideoGauss,convParam,
                gaussian,
                QT_TR_NOOP("Gauss smooth"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Gaussian smooth. Blur the picture."));

//          6 10 6
//         10 16 10 *1/80
//		    6 10 6
char 							*AVDMFastVideoGauss::printConf(void)
{
		static char str[]="Gauss(fast)";
		return (char *)str;
	
}
 uint8_t AVDMFastVideoGauss::doLine(uint8_t  *pred,
																					uint8_t *cur,
   																				uint8_t *next,
   																				uint8_t *out,
                       										uint32_t w)
                                 
{
	uint8_t a1,a2,a3;
	uint8_t b1,b2,b3;
	uint8_t c1,c2,c3;
	int32_t o;
//#define MASKED__	
#define threshold 80
	int v,r;
	
	a2=*pred++;a3=*pred++;
	b2=*cur++;b3=*cur++;
	c2=*next++;c3=*next++;
	
	*out++=b2;
	w--;
	
	while(w>1)
	{
			a1=a2;
			a2=a3;
			a3=*pred++;
			b1=b2;
			b2=b3;
			b3=*cur++;
			c1=c2;
			c2=c3;
			c3=*next++;
		
		  //
#ifdef MASKED__
		v=b2;
		r=16;
		o=b2*16;
		#define MORE(x,coef) if(abs(x-v)<=threshold) {o+=x*coef;r+=coef;}
		MORE(a1,6);
		MORE(a2,10);
		MORE(a3,6);
		
		MORE(b1,10);
		MORE(b3,10);
		
		MORE(c1,6);
		MORE(c2,10);
		MORE(c3,6);
		
		o=(o+r-1)/r;		
		
#else		  
		  o=6*a1+10*a2+6*a3+10*b1+16*b2+10*b3+6*c1+10*c2+6*c3;		  
		  o/=80;
#endif		  
		  
		  *out++=o;
		  w--;
	}	
	*out++=b3;
		return 1;
}




