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
         float *l=xv_left[j]=(float *)ADM_alloc( sizeof(float)*(NZEROS*2+2));
         float *r=xv_right[j]=(float *)ADM_alloc( sizeof(float)*(NZEROS*2+2));
         for(int i=0;i<2*(NZEROS+1);i++)
         {
             l[i]=r[i]=0;
         }

     }    
    posLeft=posRight=0;
    //printf("Dolby kernel size=%d\n",sizeof(xcoeffs)/sizeof(float));
}
ADMDolbyContext::~ADMDolbyContext()
{
     for(int j=0;j<4;j++)
     {
         float *l=xv_left[j];
         float *r=xv_right[j];
         ADM_dezalloc(l);
         ADM_dezalloc(r);
     }
    
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
   float *src1=oldie;         // Aligned also
   float *src2=coef;          // that one is always aligned to a 16 bytes boundary
    float sum = 0;
    for(int i=0;i<NZEROS+1;i++)
    {        
            sum+=src1[i]*src2[i];
    }
    return sum;
}


float ADMDolbyContext::DolbyShift_convolutionAlign3(float *oldie, float *coef)
{
   float *src1=oldie;         // Aligned also
   float *src2=coef;          // that one is always aligned to a 16 bytes boundary
    int mod16=(1+NZEROS)>>2;
    int left=(1+NZEROS)&3;
    static float __attribute__ ((__aligned__ (16))) sum16[4]={0,0,0,0};
    float sum = 0;
    for(int i=0;i<mod16;i++)
    {
        for(int x=0;x<4;x++)
            sum16[x]+=src1[x]*src2[x];
    
        src1+=4;
        src2+=4;
    }
   
    
	for (int i = 0; i <left; i++)
		sum += (*src1++)*(*src2++);
        for(int i=0;i<4;i++)
            sum+=sum16[i];
	return sum;
}
/**
  * 
  * @param isamp
  * @return 
  */
#ifdef ADM_CPU_X86
float ADMDolbyContext::DolbyShift_convolutionAlignSSE(float *oldie, float *coef)
{
     float *src1=oldie;         // Aligned also
     float *src2=coef;          // that one is always aligned to a 16 bytes boundary
    int mod16=(1+NZEROS)>>2;
    int left=(1+NZEROS)&3;
    static float __attribute__ ((__aligned__ (16))) sum16[4];
    
    float sum = 0;
     __asm__(
                        "xorps          %%xmm2,%%xmm2     \n" // carry
                        "1: \n"
                        "movaps         (%0),%%xmm0  \n" // src1
                        "movaps         (%1),%%xmm1  \n" // src2
                        "mulps          %%xmm1,%%xmm0 \n" // src1*src2
                        "addps          %%xmm0,%%xmm2 \n" // sum+=src1*src2
                        "add           $16,%0      \n"
                        "add           $16,%1      \n"
                        "sub           $1,%3      \n"
                        "jnz             1b        \n"
                        "movaps        %%xmm2,(%2)        \n"

                : : "r" (src1),"r" (src2),"r"(sum16),"r"(mod16)
                );
   
    
	for (int i = 0; i <left; i++)
		sum += (*src1++)*(*src2++);
        for(int i=0;i<4;i++)
            sum+=sum16[i];
	return sum;
}
#endif ADM_CPU_X86
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
        float sum;
        if(skip) return isamp;
        setValue(xv_left,posLeft,isamp / GAIN);
#ifdef ADM_CPU_X86
        if(CpuCaps::hasSSE())
        {
            int mod=posLeft&3;
            int of2=posLeft-mod;
            sum = DolbyShift_convolutionAlignSSE(xv_left[mod]+of2,xcoeffs);
        }else
#endif            
        {
            sum= DolbyShift_convolution(posLeft,xv_left[0],xcoeffs);
        }
//--
	posLeft++;
	if (posLeft > NZEROS)
		posLeft = 0;

	return sum;
}
/**
 * \fn DolbyShiftRight
 * @param isamp
 * @return 
 */
float ADMDolbyContext::DolbyShiftRight(float isamp)
{
        float sum;
        if(skip) return isamp;
        setValue(xv_right,posRight,isamp / GAIN);
#ifdef ADM_CPU_X86
        if(CpuCaps::hasSSE())
        {
            int mod=posRight&3;
            int of2=posRight-mod;
            sum = DolbyShift_convolutionAlignSSE(xv_right[mod]+of2,xcoeffs);
        }else
#endif            
        {
            sum= DolbyShift_convolution(posRight,xv_right[0],xcoeffs);
        }
//--
	posRight++;
	if (posRight > NZEROS)
		posRight = 0;
	return -sum;
}
