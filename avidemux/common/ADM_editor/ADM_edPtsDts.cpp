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

#define GOT_NO_PTS 2
#define GOT_NO_DTS 1
#define GOT_NONE   3
#define GOT_BOTH   0
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
        if(pts==ADM_NO_PTS) k+=GOT_NO_PTS;
        if(dts==ADM_NO_PTS) k+=GOT_NO_DTS;
        switch(k)
        {
            case GOT_BOTH : // Got both
                if(pts!=dts)
                            {
                                    printf("[Editor] Pts!=Dts for frame %"LU"\n",i);
                            }
                first=pts; // do nothing since we already have both...
                continue;            
                break;
            case GOT_NONE: // Got none
                {
                        if(first!=ADM_NO_PTS)
                        {
                            first+=timeIncrementUs; // Say this one = previous + timeIncrement
                            pts=dts=first;
                        }else
                            continue;   // We dont have a previous skip that one
                }
                break;
            case GOT_NO_DTS :  // got only pts
                first=dts=pts;
                break;
            case GOT_NO_PTS: // got only dts
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
    bool bframe=false;
    aviInfo info;
    uint32_t flags;
    hdr->getVideoInfo(&info);

    uint64_t lastPts=ADM_NO_PTS;
    uint64_t lastDts=ADM_NO_PTS;
    // Scan to see if we have b-frames
    for(int i=0;i<info.nb_frames;i++)
    {
        hdr->getFlags(i,&flags);
        if(flags & AVI_B_FRAME)
        {
            bframe=true;
            break;
        }
    }
    if(false==bframe)
    {
        ADM_info("No B frame found, settings PTS=DTS\n");
        return setPtsEqualDts(hdr,timeIncrementUs);
    }
#if 0
    for(int i=0;i<info.nb_frames;i++)
    {
        uint64_t pts,dts;
        if(true!=hdr->getPtsDts(i,&pts,&dts))
        {
            printf("[Editor] GetPtsDts failed for frame %"LU"\n",i);
            return false;
        }
        int k=0;
        if(pts==ADM_NO_PTS) k+=GOT_NO_PTS;
        if(dts==ADM_NO_PTS) k+=GOT_NO_DTS;
        switch(k)
        {
            case GOT_BOTH : // Got both
                    lastPts=pts;
                    lastDts=dts;
                continue;            
                break;
            case GOT_NONE: // Got none
                {
                        if(lastDts!=ADM_NO_PTS)
                        {
                            lastDts+=timeIncrementUs; // Say this one = previous + timeIncrement
                            dts=lastDts;
                        }else
                            continue;   // We dont have a previous skip that one
                }
                break;
            case GOT_NO_DTS :  // got only pts
                        if(lastDts!=ADM_NO_PTS)
                        {
                            lastDts+=timeIncrementUs; // Say this one = previous + timeIncrement
                            dts=lastDts;
                        }else
                            continue;   // We dont have a previous skip that one
                        //
                        ADM_assert(dts<=pts);
                // Dts from PTS
                break;
            case GOT_NO_PTS: // got only dts
                lastDts=dts;
                {
                    uint32_t flags;
                    hdr->getFlags(i,&flags);
                    if(flags & AVI_B_FRAME) pts=dts;
                }
                // Pts from DTS
                break;
            default:
                ADM_assert(0);
                break;
        }
        ADM_assert(dts<=pts);
        // update
        if(true!=hdr->setPtsDts(i,pts,dts))
        {
            printf("[Editor] SetPtsDts failed for frame %"LU"\n",i);
            return false;
        }
    }
#endif
    return true;
}
