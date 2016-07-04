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

#include "ADM_vidConvolution.hxx"
#include "convolution_desc.cpp"



DECLARE_VIDEO_FILTER_PARTIALIZABLE(   AVDMFastVideoMedian,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_NOISE,            // Category
                        "Median",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("median","Median convolution."),            // Display name
                        QT_TRANSLATE_NOOP("meadian","3x3 convolution filter :median.") // Description
                    );
/**
    \fn getConfiguration
*/

const char 							*AVDMFastVideoMedian::getConfiguration(void)
{
		static char str[]="Median(fast)";
		return (char *)str;
	
}
/**
    \fn doLine
*/
 uint8_t AVDMFastVideoMedian::doLine(uint8_t  *pred,
                                    uint8_t *cur,
                                    uint8_t *next,
                                    uint8_t *out,
                                    uint32_t w)
{
	uint8_t a1,a2,a3;
	uint8_t b1,b2,b3;
	uint8_t c1,c2,c3; //,i;
	//int32_t o;
	uint8_t temp;
	
	static uint8_t tab[9];
	a2=*pred++;a3=*pred++;
	b2=*cur++;b3=*cur++;
	c2=*next++;c3=*next++;
	
	*out++=b2;
	w--;
	
	while(w>1)
	{
			tab[0]=a1=a2;
			tab[1]=a2=a3;
			tab[2]=a3=*pred++;
			tab[3]=b1=b2;
			tab[4]=b2=b3;
			tab[5]=b3=*cur++;
			tab[6]=c1=c2;
			tab[7]=c2=c3;
			tab[8]=c3=*next++;
		
#define PIX_SORT(a,b) { if ((a)>(b)) PIX_SWAP((a),(b)); }
#define PIX_SWAP(a,b) { temp=(a);(a)=(b);(b)=temp; }

   uint8_t *p=(uint8_t *)tab;
								
    PIX_SORT(p[1], p[2]) ; PIX_SORT(p[4], p[5]) ; PIX_SORT(p[7], p[8]) ;
    PIX_SORT(p[0], p[1]) ; PIX_SORT(p[3], p[4]) ; PIX_SORT(p[6], p[7]) ;
    PIX_SORT(p[1], p[2]) ; PIX_SORT(p[4], p[5]) ; PIX_SORT(p[7], p[8]) ;
    PIX_SORT(p[0], p[3]) ; PIX_SORT(p[5], p[8]) ; PIX_SORT(p[4], p[7]) ;
    PIX_SORT(p[3], p[6]) ; PIX_SORT(p[1], p[4]) ; PIX_SORT(p[2], p[5]) ;
    PIX_SORT(p[4], p[7]) ; PIX_SORT(p[4], p[2]) ; PIX_SORT(p[6], p[4]) ;
    PIX_SORT(p[4], p[2]) ; 
		  
		  *out++=tab[4];
		  w--;
	}	
	*out++=b3;
	return 1;
}
