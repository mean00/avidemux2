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
#if 0
 REGISTERX(VF_SHARPNESS, "gaussian",QT_TR_NOOP("Gauss smooth"),QT_TR_NOOP("Gaussian smooth. Blur the picture."),VF_GAUSSIAN,1,Gaussian_create,gaussian_script);
    REGISTERX(VF_SHARPNESS, "median",QT_TR_NOOP("Median"),QT_TR_NOOP("Median kernel 3x3. Remove high frequency noise."),VF_MEDIAN,1,median_create,median_script);
#endif
VF_DEFINE_FILTER(AVDMFastVideoMean,convParam,
                mean,
                QT_TR_NOOP("Mean"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Mean (blur) kernel."));

char 	*AVDMFastVideoMean::printConf(void)
{
		static char str[]="Mean(fast)";
		return (char *)str;
	
}
uint8_t AVDMFastVideoMean::doLine(uint8_t  *pred,
																					uint8_t *cur,
   																				uint8_t *next,
   																				uint8_t *out,
                       										uint32_t w)
                                 
{
	uint8_t a1,a2,a3;
	uint8_t b1,b2,b3;
	uint8_t c1,c2,c3;
	int32_t o;
	
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
		  o=a1+a2+a3+b1+b2+b3+c1+c2+c3;
		  o/=9;
		  
		  *out++=o;
		  w--;
	}	
		*out++=b3;
		return 1;
}
