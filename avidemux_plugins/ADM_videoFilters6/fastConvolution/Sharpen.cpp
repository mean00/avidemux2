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


DECLARE_VIDEO_FILTER_PARTIALIZABLE(   AVDMFastVideoSharpen,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_SHARPNESS,            // Category
                        "Sharpen",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("sharpen","Sharpen convolution."),            // Display name
                        QT_TRANSLATE_NOOP("sharpen","3x3 convolution filter :sharpen.") // Description
                    );

/**
    \fn getConfiguration
*/

//         -1 -2 -1
 	//         -2 16 -2 *1/16
  	//		    -1 -2 -1
const char 							*AVDMFastVideoSharpen::getConfiguration(void)
{
		static char str[]="Sharpen(fast)";
		return (char *)str;
	
}
/**
    \fn doLine
*/

 uint8_t AVDMFastVideoSharpen::doLine(uint8_t  *pred,
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
		  //         -1 -2 -1
 	//         -2 16 -2 *1/16
  	//		    -1 -2 -1
		  //o=-a1+-2*a2+-a3+-2*b1+16*b2+-2*b3+-1*c1+-2*c2+-1*c3;
		  //o/=16;
		  o=4*b2-a2-c2-b1-b3;
		  o>>=2;
		  o+=3*a2;
		  o=o/3;
		  if(o<0) o=0;
		  if(o>255) o=255;
		  
		  *out++=o;
		  w--;
	}	
	*out++=b3;
		return 1;
}
// EOF
