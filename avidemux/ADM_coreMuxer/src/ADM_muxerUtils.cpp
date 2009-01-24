/***************************************************************************
                          ADM_muxerUtils.cpp  -  description
                             -------------------
    copyright            : (C) 2008 by mean
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
#include "ADM_muxerInternal.h"
#include "ADM_muxerUtils.h"
/**
    \fn rescaleFps
    \brief Rescale fps to be accurate (i.e. 23.976 become 24000/1001)

*/
void  rescaleFps(uint32_t fps1000, AVRational *rational)
{
    switch(fps1000)
    {
    case 23976 :
    {
        rational->num=1001;
        rational->den=24000;
        break;
    }
    case 29970 :
    {
        rational->num=1001;
        rational->den=30000;
        break;
    }
    default:
    rational->num=1000;
    rational->den=fps1000;
    }
    printf("[MP3] TimeBase for video %d/%d\n",rational->num,rational->den);
}
/**
        \fn rescaleLavPts
        \brief Rescale PTS/DTS the lavformat way, i.e. relative to the scale.
*/
uint64_t rescaleLavPts(uint64_t us, AVRational *scale)
{

     if(us==ADM_NO_PTS) return 0x8000000000000000LL;  // AV_NOPTS_VALUE
    double db=(double)us;
    double s=scale->den;

     db*=s;
     db=(db)/1000000.; // in seconds
    uint64_t i=(uint64_t)db; // round up to the closer num value
    i=(i+scale->num-1)/scale->num;
    i*=scale->num;
    return i;
}
