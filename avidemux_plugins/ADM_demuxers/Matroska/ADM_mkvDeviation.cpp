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

int comp64_t (const void * elem1, const void * elem2) 
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
int mkvDeviation::computeDeviation(int num, int den)
{
  double dHalf=(500000.*(double)num)/((double)den);
  int half=dHalf-1; // half interval in us
  double sumOfError=0;
  // Sorted contains the sorted list of valid PTS
  double coeff=(double)num*1000000.;
  coeff=coeff/(double)den;
  
  int lastValidFrame=1;
  for(int i=0;i<nbValid;i++)
  {
    uint64_t pts=sorted[i];
    double dmultiple=(pts+half);
    dmultiple/=coeff;
    uint64_t multiple=(uint64_t)dmultiple;
    double reconstructed=(double)multiple*coeff;
    double deviation=(double)fabs((double)pts-reconstructed);
    if(multiple<=lastValidFrame)
        printf("Warning : Multiple match for the same number (%d)\n",multiple);
    lastValidFrame=multiple;
    //printf("frame %d multiple = %d, deviation=%d\n",i,(int)multiple,(int)deviation);
    
    // We have an accuracy of 1ms, so if the error is less than 2 ms, we ignore it
    if(deviation>2000.)
        sumOfError=sumOfError+(deviation*deviation);
  }

  double scale=nbValid;
  sumOfError/=scale*scale;
  ADM_info("Den=%d Num=%d  sum of error=%f, %d\n",den,num,(float)sumOfError,(int)sumOfError);
  return (int)sumOfError  ;
}
