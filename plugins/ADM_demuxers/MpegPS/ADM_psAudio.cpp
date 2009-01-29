/***************************************************************************
    \file ADM_psAudio.cpp

    copyright            : (C) 2006/2009 by mean
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
#include "ADM_Video.h"

#include <string.h>
#include <math.h>

#include "ADM_ps.h"
/**
    \fn ADM_psAccess
*/
ADM_psAccess::ADM_psAccess(const char *name,uint8_t pid,bool append)
{
FP_TYPE fp=FP_DONT_APPEND;
        if(append) fp=FP_APPEND;
        this->pid=pid;
        if(!demuxer.open(name,&fp)) ADM_assert(0);
}

/**
    \fn ~ADM_psAccess
*/
ADM_psAccess::~ADM_psAccess()
{
    demuxer.close();
}
/**
    \fn push
    \brief add a seek point.
*/
bool      ADM_psAccess::push(uint64_t at, uint64_t dts,uint32_t size)
{
ADM_psAudioSeekPoint s;
            s.position=at;
            s.dts=dts;
            s.size=size;
            seekPoints.push_back(s);
            return true;
}
/**
    \fn getLength
*/
uint32_t  ADM_psAccess::getLength(void)
{
  return (seekPoints[seekPoints.size()-1].size);

}
/**
    \fn getDurationInUs
*/
uint64_t  ADM_psAccess::getDurationInUs(void)
{
    // Take last seek point; should be accurate enough
    return timeConvert(seekPoints[seekPoints.size()-1].dts);
}
/**
    \fn goToTime
*/                              
bool      ADM_psAccess::goToTime(uint64_t timeUs)
{
    // Convert time in us to scaled 90 kHz tick
    double f=timeUs;
    f*=90;
    f/=1000;
    f+=dtsOffset;
    uint64_t n=(uint64_t)f;

    if(n<=seekPoints[seekPoints.size()-1].dts)
    {
            demuxer.setPos(seekPoints[0].position);
            return true;
    }

    for(int i=0;i<seekPoints.size()-1;i++)
    {
        if(seekPoints[i].dts >=n && seekPoints[i+1].dts>n)
        {
            demuxer.setPos(seekPoints[i].position);
            return true;
        }
    }
    return false;
}
/**
    \fn timeConvert
    \brief Convert time in ticks raw from the stream to avidemux time in us starting from the beginning of the file
*/
uint64_t ADM_psAccess::timeConvert(uint64_t x)
{
    if(x==ADM_NO_PTS) return ADM_NO_PTS;
    x=x-dtsOffset;
    x=x*1000;
    x/=90;
    return x;

}
/**
    \fn getPacket
*/
bool      ADM_psAccess::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
uint64_t p,d,start;
    if(false==demuxer.getPacketOfType(pid,maxSize,size,&p,&d,buffer,&start)) return false;
    if(d==ADM_NO_PTS) *dts=p;
            else *dts=d;
    *dts=timeConvert(*dts);
    return true;
}


//EOF
