/***************************************************************************

 Custom IVTC

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

#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_vidMisc.h"
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "admIvtc.h"
#include "ADM_admIvtc.h"

extern bool copyField(ADMImage *target, ADMImage *source, bool top);

#if 1
#define aprintf printf
#else
#define aprintf(...) {}
#endif


/**
 * \fn lookupSync
 * \brief Try to search for a sequence
 * @return
 */
ivtcMatch  admIvtc::searchSync(int  &offset)
{
    ADMImage *images[PERIOD*2];
  
    offset=0xff;

    aprintf("Searching sync\n");

    for(int i=0;i<(PERIOD+2);i++)
    {
        images[i]=vidCache->getImage(nextFrame+i);
        if(!images[i])
        {            
            return IVTC_NO_MATCH;
        }
    }
    // if it not all NTSC, dont even try           
    
    int film=0;
    for(int i=0;i<PERIOD+1;i++)
    {
        if((images[i+1]->Pts-images[i]->Pts)> 41000) // 24 fps
            film++;
    }
    if(film)
    {
        aprintf("Not all NTSC, not even trying\n");
         return IVTC_NO_MATCH;
    }
    for(int i=0;i<PERIOD+1;i++)
    {
        matches[i]=computeMatch(images[i],images[i+1],configuration.threshold);
    }
    for(int i=0;i<PERIOD;i++)
    {
        if(matches[i]!=IVTC_NO_MATCH && matches[i+1]==IVTC_NO_MATCH && matches[i+2]!=IVTC_NO_MATCH && matches[i+2]!=matches[i])
        {
            offset=i;
            return matches[i];
        }
    }
    return IVTC_NO_MATCH;   
}

/**
 * 
 * @param fn
 * @param image
 * @return 
 */
bool admIvtc::getNextImageInSequence(uint32_t *fn,ADMImage *image)
{
    int left,right;
    uint8_t *hint=image->GetReadPtr(PLANAR_Y);
    switch(offsetInSequence)
    {

        case 1:
                left=startSequence+2;
                right=startSequence+1;                
                break;
        case 2:
                left=startSequence+3;
                right=startSequence+2;
                break;
        case 3:
        case 4:
        default:
            left=right=startSequence+offsetInSequence;            
            break;
    }
    if(mode==IVTC_TOP_MATCH)
    {
    int med=left;
        left=right;
        right=med;
    }
    ADMImage *source1=vidCache->getImage(left);
    if(!source1)
    {
        vidCache->unlockAll();
        return false;
    }
    ADMImage *source2=vidCache->getImage(right);
    if(!source2)
    {
        source2=source1;
    }
    copyField(image,source1,false);
    copyField(image,source2,true);

    if(offsetInSequence==2)
        PutHintingData(image->GetReadPtr(PLANAR_Y),MARK_DUPLICATE);
    else
        PutHintingData(image->GetReadPtr(PLANAR_Y),MARK_PROGRESSIVE);
    
    if(configuration.show)
    {
        char st[200];
        sprintf(st,"Seq=%d",offsetInSequence);
        displayStatus(image,st);
        
        for(int i=0;i<=PERIOD;i++)
        {
            sprintf(st,"%d:%d",i,matches[i]);
            image->printString(16,3+i,st);
        }


    }
    image->Pts=vidCache->getImage(offsetInSequence+startSequence)->Pts;
    offsetInSequence++;
    if(offsetInSequence>PERIOD)
        state=IVTC_RESYNCING;
     vidCache->unlockAll();
    *fn=nextFrame;
    nextFrame++;
    
    return true;               
}

#define DUPLICATE_FROM_AND_END(x)  image->duplicateFull(x);\
        vidCache->unlockAll(); \
        *fn=nextFrame; \
        nextFrame++; \
        ;


/**
 * 
 * @return 
 */
bool admIvtc::trySimpleFieldMatching()
{
    int offset;
    ivtcMatch match=searchSync(offset);
    aprintf(">>Match = %d, offset=%d,in =%d\n",match,offset,nextFrame);
    if(match!=IVTC_NO_MATCH) // Gotcha
    {
        
        offsetInSequence=1;
        startSequence=nextFrame+offset;
        mode=match;
    
        if(!offset)
        {
            state=IVTC_PROCESSING;
            aprintf("Synced mode = %d\n",mode);
            return true;
        }else
        {
            state=IVTC_SKIPPING;
            skipCount=offset;
            aprintf("Need to skip %d frames offset\n",skipCount);
            return true;
        }
    }
    return false;
}

#define INTERLACED_THRESHOLD (30*30)

/**
 * 
 * @return 
 */
uint32_t      ADMVideo_interlaceCount_C( ADMImage *top, ADMImage *bottom, int threshold,int skipFactor);
bool admIvtc::tryInterlacingDetection(ADMImage **images)
{
    int ilaced[PERIOD+1];
    for(int i=0;i<PERIOD+1;i++)
    {
        ilaced[i]=ADMVideo_interlaceCount_C(images[i],images[i],INTERLACED_THRESHOLD,configuration.mode);
        aprintf("Interlaced [%d] %d\n",i,ilaced[i]);
        
    }
    // We should have 2 interlaced and 3 progressive as P I I P P
#define MORE(a,b)     (ilaced[a]>ilaced[b])
    bool match1=MORE(1,0) && MORE(1,3) && MORE(1,4);
    bool match2=MORE(2,0) && MORE(2,3) && MORE(2,4);
    if(match1 && match2)
    {
        aprintf("Maybe IVTC pattern\n");
        // Reconstruct first interlaced image, it should not be interlaced anymore
        //copyField(spare[0],images[2],false); // bottom
        //copyField(spare[0],images[1],true); //top
        
        //copyField(spare[1],images[2],true);
        //copyField(spare[1],images[1],false);

        int top=ADMVideo_interlaceCount_C(images[1],images[2],INTERLACED_THRESHOLD,configuration.mode);
        int bottom=ADMVideo_interlaceCount_C(images[2],images[1],INTERLACED_THRESHOLD,configuration.mode);
        
        aprintf("Top = %d/%d\n",top,ilaced[1]);
        aprintf("Bottom = %d/%d\n",bottom,ilaced[1]);
     
        if(top<ilaced[1] && top<bottom)
        {
            // match
            aprintf("Match TOP \n");
            offsetInSequence=1;
            startSequence=nextFrame;
            mode=IVTC_BOTTOM_MATCH;
            state=IVTC_PROCESSING;
            return true;
        }else
            if(bottom<ilaced[1] && bottom<top)
            {
                aprintf("Match BOTTOM \n");
                offsetInSequence=1;
                startSequence=nextFrame;
                mode=IVTC_TOP_MATCH;
                state=IVTC_PROCESSING;
                return true;
            }
    }
    return false;
}

bool admIvtc::verifySamePattern(ADMImage **images, ivtcMatch candidate)
{
    bool toptop;
    if(candidate==IVTC_TOP_MATCH)
        toptop=true;
    else 
        toptop=false;
    
    int before=ADMVideo_interlaceCount_C(images[1],images[1],INTERLACED_THRESHOLD,configuration.mode);
    int before2=ADMVideo_interlaceCount_C(images[2],images[2],INTERLACED_THRESHOLD,configuration.mode);
    //copyField(spare[0],images[2],toptop);
    //copyField(spare[0],images[1],!toptop);   
    
    int after=0;
    int after2=0;
    if(toptop)
    {
        after=ADMVideo_interlaceCount_C(images[2],images[1],INTERLACED_THRESHOLD,configuration.mode);
        after2=ADMVideo_interlaceCount_C(images[3],images[2],INTERLACED_THRESHOLD,configuration.mode);
    }
    else
    {
        after=ADMVideo_interlaceCount_C(images[1],images[2],INTERLACED_THRESHOLD,configuration.mode);
        after2=ADMVideo_interlaceCount_C(images[2],images[3],INTERLACED_THRESHOLD,configuration.mode);
    }
    
    aprintf("Before1  %d, After %d\n",before,after);
    aprintf("Before2  %d, After %d\n",before2,after2);
    
    
    
    
    if((after*3 < before*2) && (after2*3 < before2 *2))
        return true;
    return false;
}

/**
 * 
 * @param fn
 * @param image
 * @return 
 */

bool admIvtc::getNextFrame(uint32_t *fn,ADMImage *image)
{
    aprintf("--------------------\nMode = %d, offsetInSequence=%d\n",state,offsetInSequence);
    if(state==IVTC_PROCESSING)
        return  getNextImageInSequence(fn,image);        
    
    // Try to find sync...
    // 1- preload cache
    ADMImage *images[PERIOD+2];
    for(int i=0;i<PERIOD+2;i++)
    {
        images[i]=vidCache->getImage(nextFrame+i);
        if(!images[i])
        {            
            if(!i)
            {
                vidCache->unlockAll();
                aprintf("Cannot get source image\n");
                return false;
            }
            DUPLICATE_FROM_AND_END(images[0]);
            aprintf("incomplete sequence\n");
            return true;
        }
    }
    
    // we have found the pattern ahead, skip
    if(state==IVTC_SKIPPING)
    {
        aprintf("Skipping %d left\n",skipCount);
        
        skipCount--;
        DUPLICATE_FROM_AND_END(images[0]);
         PutHintingData(image->GetReadPtr(PLANAR_Y),0);
        if(!skipCount)
        {
            state=IVTC_PROCESSING;
            aprintf("Swiching to processing\n");
            displayStatus(image,"SEQ 0 ");
            PutHintingData(image->GetReadPtr(PLANAR_Y),MARK_PROGRESSIVE);
        }else
            displayStatus(image,"SKIPPING");
         
         
         return true;
    }
    
    
    // is the 2 first in NTSC mode ? 30 or 60 fps ?
    bool rightFps=true;
    for(int i=0;i<PERIOD+1;i++)
    {
        int deltaPts=(int)(images[i+1]->Pts-images[i+0]->Pts);
        aprintf("Delta fps = %d \n",deltaPts);
        if((deltaPts < 33000 || deltaPts>34000) && (deltaPts < 33000*2 || deltaPts>34000*2) )
               rightFps=false;

    }
    
    if(!rightFps)
    {
        DUPLICATE_FROM_AND_END(images[0]);
        displayStatus(image,"Skipping,wrong fps");
        aprintf("Wrong fps\n");
        return true;
    }

    if(state==IVTC_RESYNCING)
    {
        if(verifySamePattern(images,mode))
        {
            aprintf("Same pattern\n");
            state=IVTC_PROCESSING;
            offsetInSequence=1;
            startSequence=nextFrame;           
            DUPLICATE_FROM_AND_END(images[0]);
            displayStatus(image,"Seq: 0, same pattern");
            PutHintingData(image->GetReadPtr(PLANAR_Y),MARK_PROGRESSIVE);
            return true;
        }else
        {            
            state=IVTC_SYNCING;
        }
               
    }
    
    // Try to find our pattern
    // first field matching, it works only if there is some movement
    
    if(!trySimpleFieldMatching())
    {
        tryInterlacingDetection(images);
    }
   
    
    ADMImage *img=vidCache->getImage(nextFrame);
    if(!img)
    {
        vidCache->unlockAll();
        return false;
    }
    DUPLICATE_FROM_AND_END(img);
    if(state==IVTC_SYNCING)
    {
        displayStatus(image,"SYNCING, not sync found");
    }else
    {
        displayStatus(image,"SEQ 0 ");
        PutHintingData(image->GetReadPtr(PLANAR_Y),MARK_PROGRESSIVE);
    }
    return true;               
}

/************************************************/
//EOF
