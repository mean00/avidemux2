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
    for(int j=0;j<4;j++)
    {
           for(int i=0;i<2*(NZEROS+1);i++)
           {
               lefty[j][i]=righty[j][i]=0;
           }
           xv_left[j]=lefty[j];
           xv_right[j]=righty[j];
    }
    
    posLeft=posRight=0;
    //printf("Dolby kernel size=%d\n",sizeof(xcoeffs)/sizeof(float));
}
 /**
  * 
  * @param isamp
  * @return 
  */
float ADMDolbyContext::DolbyShift_simple(int pos, float *oldie, float *coef)
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
float ADMDolbyContext::DolbyShift_convolution(int pos, float *oldie, float *coef)
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
float ADMDolbyContext::DolbyShift_convolutionAlign1(int pos, float *oldie, float *coef)
{
    const float *src1=oldie+pos;
    const float *src2=coef;          // that one is always aligned to a 16 bytes boundary
    float sum = 0;
	for (int i = 0; i < 1+NZEROS; i++)
		sum += (*src1++)*(*src2++);
	return sum;
}
/**
  * 
  * @param isamp
  * @return 
  */
float ADMDolbyContext::DolbyShift_convolutionAlign2(float *oldie, float *coef)
{
    const float *src1=oldie;         // Aligned also
    const float *src2=coef;          // that one is always aligned to a 16 bytes boundary
    float sum = 0;
	for (int i = 0; i < 1+NZEROS; i++)
		sum += (*src1++)*(*src2++);
	return sum;
}
  /**
   * 
   * @param target
   * @param offset
   * @param value
   * @return 
   */
bool ADMDolbyContext::setOneValue(float *target,int offset, float value)
{
    if(offset<=0)
        offset+=NZEROS+1;
    target[offset + NZEROS] = value;
    target[offset -1] = value;
}
  /**
   * 
   * @param target
   * @param offset
   * @param value
   * @return 
   */
bool ADMDolbyContext::setValue(float **target,int offset, float value)
{
    for(int i=0;i<4;i++)
    {
        setOneValue(target[i],offset-i,value);
    }
}
 /**
  * 
  * @param isamp
  * @return  
  */
float ADMDolbyContext::DolbyShiftLeft(float isamp)
{
        if(skip) return isamp;
        setValue(xv_left,posLeft,isamp / GAIN);
//--
        float sum1= DolbyShift_simple(posLeft,xv_left[0],xcoeffs);
	float sum = DolbyShift_convolution(posLeft,xv_left[0],xcoeffs);
        float sum2= DolbyShift_convolutionAlign1(posLeft,xv_left[0],xcoeffs);
        
        int mod=posLeft&3;
        int of2=posLeft-mod;
        float sum3= DolbyShift_convolutionAlign2(xv_left[mod]+of2,xcoeffs);
//--
	posLeft++;
	if (posLeft > NZEROS)
		posLeft = 0;

        if(sum1!=sum)
        {
            ADM_warning("Mismatch!\n");
            exit(-1);
        }
        if(sum1!=sum2)
        {
            ADM_warning("Aligned 1 Mismatch!\n");
            exit(-1);
        }
         if(sum1!=sum3)
        {
            ADM_warning("Aligned 2 Mismatch!\n");
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
		xv_right[0][NZEROS] = isamp / GAIN;
	else
		xv_right[0][posRight - 1] = isamp / GAIN;

	float sum = 0;
	for (int i = posRight; i <= NZEROS; i++)
		sum += (*(p_xcoeffs++) * xv_right[0][i]);

	for (int i = 0; i < posRight; i++)
		sum += (*(p_xcoeffs++) * xv_right[0][i]);

	posRight++;
	if (posRight > NZEROS)
		posRight = 0;

	return -sum;
}
