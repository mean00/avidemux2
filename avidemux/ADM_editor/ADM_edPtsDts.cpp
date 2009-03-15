/***************************************************************************

    \file ADM_edPtsDts.h
    \brief Try to guess Pts from Dts. Mostly used for crappy format like avi.

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
#include "math.h"

#include "ADM_edPtsDts.h"

/**
    \fn setPtsEqualDts
    \brief for Low delay codec, set PTS=DTS, fill the missing values
*/
bool setPtsEqualDts(vidHeader *hdr,uint64_t timeIncrementUs)
{
    aviInfo info;
    hdr->getVideoInfo(&info);
    uint64_t first=ADM_NO_PTS;
    for(int i=0;i<info.nb_frames;i++)
    {
        uint64_t pts,dts;
        if(true!=hdr->getPtsDts(i,&pts,&dts))
        {
            printf("[Editor] GetPtsDts failed for frame %"LU"\n",i);
            return false;
        }
        int k=0;
        if(pts==ADM_NO_PTS) k+=2;
        if(dts==ADM_NO_PTS) k+=1;
        switch(k)
        {
            case 0 : // Got both
                if(pts!=dts)
                            {
                                    printf("[Editor] Pts!=Dts for frame %"LU"\n",i);
                            }
                first=pts; // do nothing since we already have both...
                continue;            
                break;
            case 3: // Got none
                {
                        if(first!=ADM_NO_PTS)
                        {
                            first+=timeIncrementUs; // Say this one = previous + timeIncrement
                            pts=dts=first;
                        }else
                            continue;   // We dont have a previous skip that one
                }
                break;
            case 2 :  // got only pts
                    first=dts=pts;
                    break;
            case 1: // got only dts
                first=pts=dts;
                break;
            default:
                ADM_assert(0);
                break;
        }
        // update
        if(true!=hdr->setPtsDts(i,pts,dts))
        {
            printf("[Editor] SetPtsDts failed for frame %"LU"\n",i);
            return false;
        }
    }
    return true;
}
/**
    \fn setMpeg4PtsFromDts
    \brief For mpeg4 SP/ASP, recompute PTS DTS using the simple I/P/B frame reordering
    Works also for mpeg1/2
    It absolutely NEEDS to have the proper frame type set (PTS/DTS/...)
*/
bool setMpeg4PtsFromDts(vidHeader *hdr,uint64_t timeIncrementUs)
{
    return true;
}
