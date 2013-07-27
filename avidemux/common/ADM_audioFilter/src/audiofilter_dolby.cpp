/**
// \file audiofilter_dolby.cpp
//
// Description: 
//
//
// Author: Mihail Zenkov <kreator@tut.by>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "audiofilter_dolby.h"
#include "audiofilter_dolby_table.h"

bool ADMDolbyContext::skip=false;

void ADMDolbyContext::DolbySkip(bool on)
{
	skip = on;
        
}
/**
 * 
 */
 ADMDolbyContext::ADMDolbyContext()
        {
            for(int i=0;i<2*(NZEROS+1);i++)
            {
                xv_left[i]=xv_right[i]=0;
            }
            posLeft=posRight=0;
            //printf("Dolby kernel size=%d\n",sizeof(xcoeffs)/sizeof(float));
        }
 /**
  * 
  * @param isamp
  * @return 
  */
float ADMDolbyContext::DolbyShiftLeft_simple(int pos, float *oldie, float *coef)
{

	float sum = 0;
	for (int i = pos; i <= NZEROS; i++)
		sum += (*(coef++) * oldie[i]);

	for (int i = 0; i < pos; i++)
		sum += (*(coef++) * oldie[i]);
        return sum;
}
/**
  * 
  * @param isamp
  * @return 
  */
float ADMDolbyContext::DolbyShiftLeft_convolution(int pos, float *oldie, float *coef)
{

	float sum = 0;
	for (int i = pos; i < pos+1+NZEROS; i++)
		sum += (*(coef++) * oldie[i]);
	return sum;
}
  
 /**
  * 
  * @param isamp
  * @return  
  */
float ADMDolbyContext::DolbyShiftLeft(float isamp)
{
if(skip) return isamp;

	float *p_xcoeffs = xcoeffs;


	if (!posLeft )
        {
		xv_left[NZEROS+NZEROS+1] = isamp / GAIN;
                xv_left[NZEROS] = isamp / GAIN;
        }
	else
        {
		xv_left[posLeft + NZEROS] = isamp / GAIN;
                xv_left[posLeft -1] = isamp / GAIN;
        }
//--
        float sum1= DolbyShiftLeft_simple(posLeft,xv_left,xcoeffs);
	float sum = DolbyShiftLeft_convolution(posLeft,xv_left,xcoeffs);
//--
	posLeft++;
	if (posLeft > NZEROS)
		posLeft = 0;

        if(sum1!=sum)
        {
            ADM_warning("Mismatch!\n");
            exit(-1);
        }
	return sum;
}
/**
 * \fn DolbyShiftRight
 * @param isamp
 * @return 
 */
float ADMDolbyContext::DolbyShiftRight(float isamp)
{
if(skip) return isamp;
	float *p_xcoeffs = xcoeffs;
	

	if ((posRight - 1) < 0)
		xv_right[NZEROS] = isamp / GAIN;
	else
		xv_right[posRight - 1] = isamp / GAIN;

	float sum = 0;
	for (int i = posRight; i <= NZEROS; i++)
		sum += (*(p_xcoeffs++) * xv_right[i]);

	for (int i = 0; i < posRight; i++)
		sum += (*(p_xcoeffs++) * xv_right[i]);

	posRight++;
	if (posRight > NZEROS)
		posRight = 0;

	return -sum;
}
