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
#include <math.h>

#if defined( ADM_CPU_X86)
extern "C"     void adm_dolby_asm_sse2(float *src1,float *src2,float *sum, int count);
void           testDolbyAsm();
#endif


bool ADMDolbyContext::skip=false;

void ADMDolbyContext::DolbySkip(bool on)
{
	skip = on;
        
}
/**
 * 
 */
void  ADMDolbyContext::reset()
{
     for(int j=0;j<4;j++)
     {
         float *l=xv_left[j];
         float *r=xv_right[j];
         for(int i=0;i<2*(NZEROS+1);i++)
         {
             l[i]=r[i]=0;
         }

     }    
    posLeft=posRight=0;
}
/**
 * 
 */
 ADMDolbyContext::ADMDolbyContext()
{
     for(int j=0;j<4;j++)
     {
         xv_left[j]=(float *)ADM_alloc( sizeof(float)*(NZEROS*2+2));
         xv_right[j]=(float *)ADM_alloc( sizeof(float)*(NZEROS*2+2));
     }    
     reset();
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
    static float ASM_ALIGNED(16) sum16[4] = { 0,0,0,0 };
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
    static float ASM_ALIGNED(16)  sum16[4];
    
    adm_dolby_asm_sse2(src1,src2,sum16,mod16);    
    float sum = 0;

    
	for (int i = 0; i <left; i++)
		sum += (*src1++)*(*src2++);
        for(int i=0;i<4;i++)
            sum+=sum16[i];
	return sum;
}
#endif //ADM_CPU_X86
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
    return true;
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
    return true;
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
        if(CpuCaps::hasSSE2())
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
        if(CpuCaps::hasSSE2())
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

#if defined( ADM_CPU_X86)
void testDolbyAsm()
{
    float samples[512];
    for(int i=0;i<512;i++) samples[i]=((float)i)/512.;
    
    float sum1 = ADMDolbyContext::DolbyShift_convolutionAlignSSE(samples,xcoeffs);
    float sum2 = ADMDolbyContext::DolbyShift_convolution(0,samples,xcoeffs);
    
    ADM_info("SSE = %f, C = %f\n",sum1,sum2);
    if(fabs(sum1-sum2)>0.0001)
    {
        ADM_error("FAILED at line %d\n",__LINE__);
        exit(-1);
    }
    
    for(int i=16;i<512+16;i++) 
        samples[i-16]=((float)((i*4)&511))/512.;
    sum1 = ADMDolbyContext::DolbyShift_convolutionAlignSSE(samples,xcoeffs);
    sum2 = ADMDolbyContext::DolbyShift_convolution(0,samples,xcoeffs);
    
    ADM_info("SSE = %f, C = %f\n",sum1,sum2);
    if(fabs(sum1-sum2)>0.0001)
    {
        ADM_error("FAILED at line %d\n",__LINE__);
        exit(-1);
    }
    ADM_info("** PASS **\n");
    
}
#endif
