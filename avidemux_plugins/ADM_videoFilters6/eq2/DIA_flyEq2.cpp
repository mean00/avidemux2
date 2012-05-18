/***************************************************************************
                          ADM_guiContrast.cpp  -  description
                             -------------------
    begin                : Mon Sep 23 2002
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
#include "DIA_flyDialog.h"
#include "ADM_default.h"
#include "ADM_image.h"

#include "ADM_vidEq2.h"

#include "DIA_flyEq2.h"
typedef void lutMeType(oneSetting *par, ADMImage *i,ADMImage *o,ADM_PLANE plane);

/************* COMMON PART *********************/
uint8_t  flyEq2::update(void)
{
    return 1;
}
/**
    \fn process
*/
uint8_t    flyEq2::processYuv(ADMImage* in, ADMImage *out)
{
	Eq2Settings mySettings;

#if 0
	printf("Contrast   :%f\n",param.contrast);
	printf("brightness :%f\n",param.brightness);
	printf("saturation :%f\n",param.saturation);
	
	printf("gamma_weight :%f\n",param.gamma_weight);
	printf("gamma :%f\n",param.gamma);
	
	
	
	printf("rgamma :%f\n",param.rgamma);
	printf("bgamma :%f\n",param.bgamma);
	printf("ggamma :%f\n",param.ggamma);
	printf("******************\n");
#endif	
	
	        update_lut(&mySettings,&param);
	        


			lutMeType *lutMe=apply_lut;
			

#ifdef ADM_CPU_X86
	        if(CpuCaps::hasMMX())
	        {
	        		lutMe=affine_1d_MMX;
	        }
#endif	
	        lutMe(&(mySettings.param[0]),in,out,PLANAR_Y);
            lutMe(&(mySettings.param[2]),in,out,PLANAR_U);
            lutMe(&(mySettings.param[1]),in,out,PLANAR_V);
	        
	        	
#if 1
	        in->copyLeftSideTo(out);
#endif
		return 1;
}

/************* COMMON PART *********************/
