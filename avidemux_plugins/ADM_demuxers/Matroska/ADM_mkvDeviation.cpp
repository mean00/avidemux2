/***************************************************************************
    copyright            : (C) 2017 by mean
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
#include "ADM_cpp.h"
#include <math.h>
#include <stdlib.h>
#include "ADM_default.h"

#include "ADM_mkvDeviation.h"


mkvDeviation::mkvDeviation(int n)
{
    total=n;
    nbValid=0;
    sorted=new uint64_t[n];
}
mkvDeviation::~mkvDeviation()
{
    delete [] sorted;
    sorted=NULL;    
}

static int comp64_t (const void * elem1, const void * elem2)
{
    uint64_t left=*(uint64_t *)elem1;
    uint64_t right=*(uint64_t *)elem2;
    if(left==right) return 0;
    if(left>right) return 1;
    return -1;
}
void mkvDeviation::sort()
{
    qsort(sorted,nbValid,sizeof(uint64_t),comp64_t);
}


    
/**
    \fn computeDeviation
    \brief returns the # of errors when forcing timestamp to be num/den (interval in us)
      num=1000
      den=24000 for a 24 fps


*/
int mkvDeviation::computeDeviation(int num, int den,int &nbSkipped)
{
  double dHalf=(500000.*(double)num)/((double)den);
  int half=dHalf-1; // half interval in us
  double sumOfError=0;
  // Sorted contains the sorted list of valid PTS
  double coeff=(double)num*1000000.;
  coeff=coeff/(double)den;
  int multipleError=0;
  int lastValidFrame=1;
  
  int multipleCount=5;
  int maxDelta=0;
  int minDelta=8*1000 *1000;
  nbSkipped=0;
  
  for(int i=1;i<nbValid;i++)
  {
      int delta=(int)(sorted[i]-sorted[i-1]);
      if(delta>maxDelta) maxDelta=delta;
      if(delta<minDelta) minDelta=delta;
      if(sorted[i]<=sorted[i-1])
          ADM_warning("Sorting error : [%d] %lld : %lld\n",i,sorted[i],sorted[i-1]);
      
  }
      
  
  for(int i=2;i<nbValid;i++) // skip the 2 first frames, often wrong
  {
    uint64_t pts=sorted[i];
    double dmultiple=(pts+half);
    dmultiple/=coeff;
    uint64_t multiple=(uint64_t)dmultiple;
    double reconstructed=(double)multiple*coeff;
    double deviation=(double)fabs((double)pts-reconstructed);
    
    if(multiple<=lastValidFrame )
    {
     //   printf("Warning : Multiple match for the same number (%d)\n",multiple);
        sumOfError+=coeff*coeff; // full frame error
        multipleError++;
        if(multipleCount)
        {
            multipleCount--;
            printf("Frame %d, multiple = %llu\n",i,multiple);
        }
        continue;
    }
    int jump=(multiple-lastValidFrame)-1;
    if(jump)
    {
        //printf("Skipped %d %d\n",i,jump);
        nbSkipped+=jump;
        //sumOfError=sumOfError+(1+jump)*(1+jump)*coeff*coeff;
        lastValidFrame=multiple;
        continue;
    }
    lastValidFrame=multiple;
    //printf("frame %d multiple = %d, deviation=%d\n",i,(int)multiple,(int)deviation);
    
    // We have an accuracy of 1ms, so if the error is less than 2 ms, we ignore it
    if(deviation>2000.)
    {
        int dev=(int)deviation;
        dev=dev-dev%1000;
        deviation=dev;
        sumOfError=sumOfError+(deviation*deviation);
    }
  }

  sumOfError/=(double)nbValid;
  sumOfError=sqrt(sumOfError);
  ADM_info("Den=%d Num=%d  sum of error=%d, multiple=%d\n",den,num,(int)sumOfError,multipleError);
  ADM_info("MinDelta=%d maxDelta=%d skipped=%d\n",minDelta,maxDelta,nbSkipped);
  return (int)sumOfError  ;
}
