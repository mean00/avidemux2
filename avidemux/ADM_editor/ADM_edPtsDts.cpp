/***************************************************************************

    \file ADM_edPtsDts.h
    \brief Try to guess Pts from Dts

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
        if(pts==ADM_NO_PTS && dts==ADM_NO_PTS)
        {
            if(first!=ADM_NO_PTS)
            {
                first+=timeIncrementUs;
            }
            continue;
        }else
        if(dts!=ADM_NO_PTS && pts!=ADM_NO_PTS)
        {
            if(pts!=dts)
            {
                    printf("[Editor] Pts!=Dts for frame %"LU"\n",i);
                    first=pts;
                    continue;
            }
        }else
            if(pts!=ADM_NO_PTS) first=dts=pts;
            else
                first=pts=dts;
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
*/
bool setMpeg4PtsFromDts(vidHeader *hdr,uint64_t timeIncrementUs)
{
    return true;
}
