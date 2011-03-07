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
#include "ADM_cpp.h"
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
    \fn ADM_setH264MissingPts(vidHeader *hdr,uint64_t timeIncrementUs,uint64_t *delay)
    \brief look for AVC case where 1 field has PTS, the 2nd one is Bottom without PTS
            in such a case PTS (2nd field)=PTS (First field)+timeincrement
*/
bool ADM_setH264MissingPts(vidHeader *hdr,uint64_t timeIncrementUs,uint64_t *delay)
{
    aviInfo info;
    hdr->getVideoInfo(&info);
    uint32_t nbFrames=0;
    nbFrames=info.nb_frames;
    uint32_t flags,flagsNext;
    uint64_t pts,dts;
    uint64_t ptsNext,dtsNext;
    uint32_t fail=0;
    // Scan if it is the scheme we support
    // i.t. interlaced with Top => PTS, Bottom => No Pts
    // Check we have all PTS...
    fail=0;
    for(int i=0;i<nbFrames;i++)
    {
          hdr->getPtsDts(i,&pts,&dts);
          if(pts==ADM_NO_PTS) 
            {
                fail++;
              //  ADM_info("Pts for frame %"LLU"/%"LLU"is missing\n",i,nbFrames);
            }
    }
    ADM_info("We have %d missing PTS\n",fail);
    if(!fail) return true; // Have all PTS, ok...
    ADM_info("Some PTS are missing, try to guess them...\n");
    fail=0;
    //
    for(int i=0;i<nbFrames-1;i+=2)
    {
        hdr->getFlags(i,&flags);
        hdr->getFlags(i+1,&flagsNext);
        hdr->getPtsDts(i,&pts,&dts);
        hdr->getPtsDts(i+1,&ptsNext,&dtsNext);
        if(!(flagsNext & AVI_BOTTOM_FIELD)) 
        {
            fail++; 
            continue;
        }
        if(pts==ADM_NO_PTS)
        {
            fail++;
            continue;
        }
    }
    ADM_info("H264 AVC scheme: %"LU"/%"LU" failures.\n",fail,nbFrames/2);
    if(fail) goto nextScheme;
    {
    ADM_info("Filling 2nd field PTS\n");
    uint32_t fixed=0;
    for(int i=0;i<nbFrames-1;i+=2)
    {
        hdr->getFlags(i,&flags);
        hdr->getFlags(i+1,&flagsNext);
        hdr->getPtsDts(i,&pts,&dts);
        hdr->getPtsDts(i+1,&ptsNext,&dtsNext);
        if(ptsNext==ADM_NO_PTS)
        {
            ptsNext=pts+timeIncrementUs;
            hdr->setPtsDts(i+1,ptsNext,dtsNext);
            fixed++;
        }
    }
    ADM_info("Fixed %d PTS\n",fixed);
    }
nextScheme:
    fail=0;
    vector <int> listOfMissingPts;
    for(int i=0;i<nbFrames;i++)
    {
          hdr->getPtsDts(i,&pts,&dts);
          if(pts==ADM_NO_PTS) 
            {
                fail++;
                listOfMissingPts.push_back(i);
            }
    }
    // 2nd part: Try to really guess the missing ones by spotting holes...
    int n=listOfMissingPts.size();
    if(n)
    {
        ADM_info("We still have %d missing PTS\n",n);
    }
    return true;
}
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
    uint32_t nbFields=0,nbFrames=0;
    // Look how much bframes + how much valid PTS/DTS we have....
    for(int i=0;i<info.nb_frames;i++)
    {
        hdr->getFlags(i,&flags);
        if(flags & AVI_FIELD_STRUCTURE) 
                    nbFields++;
            else 
                    nbFrames++;
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
        ADM_info("We have %"LU" fields and %"LU" frames\n",nbFields,nbFrames);
        if(nbFields>2)
        {
            ADM_info("Cannot recompute PTS/DTS for field encoded picture.\n");
            return true;
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
        // No b frames, PTS=DTS
        if(!nbB)
        {
            ADM_info("No bframe, set pts=dts\n");
            delay=0;
            return setPtsEqualDts(hdr,timeIncrementUs);
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
        if(flags & AVI_B_FRAME) nbBframe++;
        else        
            {
                if(nbBframe>maxBframe) maxBframe=nbBframe;
                nbBframe=0;
            }
    }
    nbBframe=0;
    // We have now maxBframe = max number of Bframes in sequence
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
              hdr->getPtsDts(i,&fwdPts,&fwdDts);
              oldPts=fwdDts;
              hdr->setPtsDts(last,oldPts,oldDts);
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
    aviInfo info;
    uint64_t offset,pts,dts;
    uint32_t nbFrames;
    uint64_t myDelay=0;
    *delay=0;

    hdr->getVideoInfo(&info);
    nbFrames=info.nb_frames;
    // Search valid DTS
    int index=-1;
    for(int i=0;i<nbFrames;i++)
    {
        hdr->getPtsDts(i,&pts,&dts);
        if(dts!=ADM_NO_PTS) 
        {
            index=i;
            break;
        }
    }
    if(index==-1)
    {
        ADM_info("No dts found, aborting\n");
        return false;
    }
    if(index*timeIncrementUs > dts)
    {
        myDelay=index*timeIncrementUs-dts;
    }
    dts=dts+myDelay-index*timeIncrementUs;
    uint64_t curDts=dts; // First DTS...

    ADM_info("Computing missing DTS\n");

    int updated=0;
    // Special case, for 24/1001 video try to detect them
    // And alter timeIncrement accordingly
    int backIndex=-1;
    uint64_t backDts=ADM_NO_PTS;
    uint64_t increment24fps=41708;
    int nbFixup=0;
    if(timeIncrementUs>33300 && timeIncrementUs<33400) // 29.997 fps
    {
        for(int i=0;i<nbFrames;i++)
        {
            hdr->getPtsDts(i,&pts,&dts);
            if(dts!=ADM_NO_PTS)
            {
                if(backIndex!=-1)
                {
                    double deltaFrame=i-backIndex;
                    if(deltaFrame>1)
                    {
                        double deltaTime=dts-backDts;
                        deltaTime=deltaTime/(deltaFrame);
                        if(deltaTime>41700 & deltaTime<41800)
                        {
                            for(int j=backIndex+1;j<i;j++)
                            {
                                hdr->setPtsDts(j,ADM_NO_PTS,backDts+(j-backIndex)*increment24fps);
                            }
                            nbFixup+=i-backIndex; 
                        }
                    }
                }
                backIndex=i;
                backDts=dts;
            }
        }
        ADM_info("Fixed %d/%d frames as 24 fps\n",nbFixup,nbFrames);
    }


    // Fill-in the gaps
    for(int i=0;i<nbFrames;i++)
    {
        hdr->getPtsDts(i,&pts,&dts);
        //printf("Frame %d dts=%"LLU,i,dts);
        if(dts!=ADM_NO_PTS) dts+=myDelay;
        if(dts==ADM_NO_PTS) 
        {
            dts=curDts;
            updated++;
        }
        hdr->setPtsDts(i,pts,dts);
        //printf("new dts=%"LLU"\n",dts);
        curDts=dts+timeIncrementUs;
    }
    ADM_info("Updated %d Dts, now computing PTS\n",updated);
    
    bool r= setPtsFromDts(hdr,timeIncrementUs,delay);
    *delay+=myDelay;
    return r;
}
//EOF
