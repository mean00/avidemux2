/***************************************************************************
                          \fn ADM_coreVideoEncoder
                          \brief Base class for video encoder plugin
                             -------------------
    
    copyright            : (C) 2002/2009 by mean
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
#include "ADM_coreVideoEncoder.h"
extern "C" 
{
#include "ADM_lavcodec.h"
}
/**
    \fn ADM_coreVideoEncoder
*/                          
ADM_coreVideoEncoder::ADM_coreVideoEncoder(ADM_coreVideoFilter *src)
{
    source=src;
    image=NULL;
    encoderDelay=0;
}

/**
    \fn ADM_coreVideoEncoder
*/                          
ADM_coreVideoEncoder::~ADM_coreVideoEncoder()
{
    if(image) delete image;
    image=NULL;
}
typedef struct
{
    uint64_t mn,mx;
    int n,d;

}TimeIncrementType;

TimeIncrementType fpsTable[]=
{
    {  40000,40000,1 ,25},
    {  20000,20000,1 ,50},
    {  33360,33369,1001,30000},
    {  41700,41710,1001,24000},
}; 

/**
    \fn usSecondsToFrac
    \brief Convert a duration in useconds into Rationnal
*/
bool usSecondsToFrac(uint64_t useconds, int *n,int *d)
{
    // First search for known value...
    int nb=sizeof(fpsTable)/sizeof(TimeIncrementType);
    for(int i=0;i<nb;i++)
    {
        TimeIncrementType *t=fpsTable+i;
        if( useconds>=t->mn && useconds<=t->mx)
        {
            *n=t->n;
            *d=t->d;
            return true;
        }
    }
    int nn,dd;
    av_reduce(&nn,&dd, useconds, 1000000, 0xFFF0); // mpeg4 allows a maximum of 1<<16-1 as time base, should be enough for most case
    ADM_info("%"LLU" us -> %d / %d (old)\n",useconds,nn,dd);
    *n=nn;
    *d=dd;

    return true;
}

/**
    \fn getRealPtsFromInternal
    \brief Lookup in the stored value to get the exact pts from the truncated one 
*/
bool ADM_coreVideoEncoder::getRealPtsFromInternal(uint64_t val,uint64_t *dts,uint64_t *pts)
{
    int n=mapper.size();
    for(int i=0;i<n;i++)
    {
        if(mapper[i].internalTS==val)
        {
            *pts=mapper[i].realTS;
            mapper.erase(mapper.begin()+i);
            // Now get DTS, it is min (lastDTS+inc, PTS-delay)
            ADM_assert(queueOfDts.size());
            *dts=queueOfDts[0];
            queueOfDts.erase(queueOfDts.begin());
            if(*dts>*pts)
            {
                ADM_warning("Dts>Pts, that can happen if there are holes in the source, fixating..\n");
                *dts=*pts;
            }
            return true;
        }
    }
    ADM_warning("Cannot find PTS : %"LLU"\n",val);  
    for(int i=0;i<n;i++) ADM_warning("%d : %"LLU"\n",i,mapper[i].internalTS);
    ADM_assert(0);
    return false;

}
// EOF