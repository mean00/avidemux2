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
/* Specific cases */
static bool setDtsFromPts(vidHeader *hdr,uint64_t timeIncrementUs,uint64_t *delay);
static bool setPtsFromDts(vidHeader *hdr,uint64_t timeIncrementUs,uint64_t *delay);
static bool updatePtsAndDts(vidHeader *hdr,uint64_t timeIncrementUs,uint64_t *delay);
/**
    \fn ADM_computeMissingPtsDts

*/
bool ADM_computeMP124MissingPtsDts(vidHeader *hdr,uint64_t timeIncrementUs,uint64_t *delay)
{
    aviInfo info;
    uint32_t flags;
    hdr->getVideoInfo(&info);
    uint32_t nDts=0,nPts=0;
    uint32_t nbB=0;
    uint64_t pts,dts;
    // Look how much bframes + how much valid PTS/DTS we have....
    for(int i=0;i<info.nb_frames;i++)
    {
        hdr->getFlags(i,&flags);
        if(flags & AVI_B_FRAME)
                nbB++;
        if(true!=hdr->getPtsDts(i,&pts,&dts))
        {
            goto next;
        }
        if(pts!=ADM_NO_PTS)
            nPts++;
        if(dts!=ADM_NO_PTS)
            nDts++;
    }
next:
        ADM_info("Out of %"LD" frames, we have %"LD" valid DTS and %"LD" valid PTS\n",info.nb_frames,nDts,nPts);
        ADM_info("We also have %"LD" bframes\n",nbB);
        // No b frames, PTS=DTS
        if(!nbB)
        {
            delay=0;
            return setPtsEqualDts(hdr,timeIncrementUs);
        }
        // Case 1 : We have both, nothing to do
        if(nDts>=info.nb_frames-2 && nPts>=info.nb_frames-2)
         {
                ADM_info("Nothing to do\n");
                *delay=0;
                return true;
         }
        // Case 2: We have PTS but not DTS
        if(nPts>=info.nb_frames-2 )
        {
            ADM_info("Got PTS, compute dts\n");
            return setDtsFromPts(hdr,timeIncrementUs,delay);
        }
        // Case 3: We have DTS but not PTS
        if(nDts>=info.nb_frames-2 )
        {
            ADM_info("Got DTS, compute  PTS\n");
            return setPtsFromDts(hdr,timeIncrementUs,delay);
        }
        // Case 4: We have a bit of both
        ADM_info("Get some dts and pts\n");
        return updatePtsAndDts(hdr,timeIncrementUs,delay);
}

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
    \fn setDtsFromPts
    \brief For mpeg4 SP/ASP, recompute PTS DTS using the simple I/P/B frame reordering
    Works also for mpeg1/2
    It absolutely NEEDS to have the proper frame type set (PTS/DTS/...)
*/
bool setPtsFromDts(vidHeader *hdr,uint64_t timeIncrementUs,uint64_t *delay)
{
    int last=0;
    int nbFrames;
    int nbBframe=0;
    int maxBframe=0;
    uint32_t flags;
    int nbB=0;
    uint64_t pts,dts;

    aviInfo info;
    hdr->getVideoInfo(&info);
    nbFrames=info.nb_frames;
    for(int i=1;i<nbFrames;i++)
    {
        hdr->getFlags(i,&flags);
        if(flags & AVI_B_FRAME)
                nbB++;
        if(true!=hdr->getPtsDts(i,&pts,&dts))
        {
                    ADM_warning("Cannot get PTS/DTS\n");
                    return false;
        }
        if(flags & AVI_B_FRAME) nbBframe++;
        else        
            {
                if(nbBframe>maxBframe) maxBframe=nbBframe;
                nbBframe=0;
            }
    }

    for(int i=1;i<nbFrames;i++)
    {
        hdr->getFlags(i,&flags);
        hdr->getPtsDts(i,&pts,&dts);
        if(flags & AVI_B_FRAME)
        {
            pts=dts;
            hdr->setPtsDts(i,pts,dts);
            nbBframe++;
        }
        else
        {
            uint64_t oldPts,oldDts;
            uint64_t fwdPts,fwdDts;
              hdr->getPtsDts(last,&oldPts,&oldDts);
              hdr->getPtsDts(nbBframe+last+1,&fwdPts,&fwdDts);
              oldPts=fwdDts;
              hdr->setPtsDts(last,oldPts,oldDts);
            nbBframe=0;
            last=i;
        }
    }
    return 1;
}

/**
    \fn setDtsFromPts
    \brief 
*/
bool setDtsFromPts(vidHeader *hdr,uint64_t timeIncrementUs,uint64_t *delay)
{
    *delay=0;
    int last=0;
    int nbFrames;
    int nbBframe=0;
    int maxBframe=0;
    uint32_t flags;
    int nbB=0;
    uint64_t pts,dts;

    aviInfo info;
    hdr->getVideoInfo(&info);
    nbFrames=info.nb_frames;
    for(int i=1;i<nbFrames;i++)
    {
        hdr->getFlags(i,&flags);
        if(true!=hdr->getPtsDts(i,&pts,&dts))
        {
                    ADM_warning("Cannot get PTS/DTS\n");
                    return false;
        }
        if(flags & AVI_B_FRAME)
        {
                
                nbB++;
                dts=pts;
                hdr->setPtsDts(i,pts,dts);
                continue;
        }
   
        uint64_t oldPts,oldDts;
        uint64_t fwdPts,fwdDts;
          hdr->getPtsDts(last,&oldPts,&oldDts);
          dts=oldPts;
          hdr->setPtsDts(i,pts,dts);
        nbBframe=0;
        last=i;
    }
    return 1;
}

/**
    \fn setPtsFromDts
    \brief Fill in the missing PTS DTS. We got some but not all.
*/
bool updatePtsAndDts(vidHeader *hdr,uint64_t timeIncrementUs,uint64_t *delay)
{
    *delay=0;
    ADM_error("SetPtsFromDts not implemented\n");
    return true;
}
//EOF
