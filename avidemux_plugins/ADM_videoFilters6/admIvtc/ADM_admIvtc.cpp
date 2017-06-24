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
#include "admIvtc_desc.cpp"

#include "ADM_admIvtc.h"
#if defined( ADM_CPU_X86) && !defined(_MSC_VER)
        #define CAN_DO_INLINE_X86_ASM
#endif

#if 1
#define aprintf printf
#else
#define aprintf(...) {}
#endif


// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   admIvtc,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "admIvtc",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("admIvtc","ADM ivtc."),            // Display name
                        QT_TRANSLATE_NOOP("admIvtc","All in one ivtc.") // Description
                    );

// Now implements the interesting parts
/**
    \fn admIvtc
    \brief constructor
*/
admIvtc::admIvtc(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilterCached(11,in,setup)
{
    if(!setup || !ADM_paramLoad(setup,dupeRemover_param,&configuration))
    {
        // Default value
        configuration.threshold=3;
        configuration.show=false;
        configuration.mode=1; // fast!

    }
    myName="admIvtc";

    incomingNum=0;
    currentNum=0;
    phaseStart=0;
    dupeOffset=0;
    startSequence=0;
    state=IVTC_SYNCING;
}
/**
    \fn admIvtc
    \brief destructor
*/
admIvtc::~admIvtc()
{
}

/**
 * \fn lookupSync
 * \brief Try to search for a sequence
 * @return
 */
ivtcMatch  admIvtc::searchSync(int  &offset)
{
    ADMImage *images[PERIOD*2];
    ivtcMatch matches[PERIOD*2];
    offset=0xff;

    aprintf("Searching sync\n");

    for(int i=0;i<(PERIOD+2);i++)
    {
        images[i]=vidCache->getImage(incomingNum+i);
        if(!images[i])
        {            
            return IVTC_NO_MATCH;
        }
    }
    // if it not all NTSC, dont even try           
    
    int film=0;
    for(int i=0;i<PERIOD;i++)
    {
        delta[i]=0;
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
 * @param img
 * @return
 */
bool admIvtc::postProcess(ADMImage *in,ADMImage *out,uint64_t newPts)
{
   
    return true;
}


/**
    \fn copyField
*/
static bool copyField(ADMImage *target, ADMImage *source, bool top)
{
    for(int i=0;i<3;i++)
    {
        ADM_PLANE plane=(ADM_PLANE )i;
        uint8_t *dest=target->GetWritePtr(plane);
        uint8_t *src=source->GetReadPtr(plane);

        uint32_t sPitch=source->GetPitch(plane);
        uint32_t dPitch=target->GetPitch(plane);
        
        if(false==top)
        {
            dest=dest+dPitch;
            src=src+sPitch;
        }


        uint32_t h=target->GetHeight(plane);
        uint32_t w=target->GetWidth(plane);

        // copy one line out of two
        h>>=1;
        dPitch*=2;
        sPitch*=2;

        BitBlit(dest,dPitch,src,sPitch,w,h);

    }
    return true;
}



bool admIvtc::getNextFrame(uint32_t *fn,ADMImage *image)
{
    aprintf("--------------------\nMode = %d, offsetInSequence=%d\n",state,offsetInSequence);
    switch(state)
    {
        case IVTC_SYNCING:
        {
            int offset;
            ivtcMatch match=searchSync(offset);
            aprintf(">>Match = %d, offset=%d,in =%d, out=%d\n",match,offset,incomingNum,currentNum);
            if(match!=IVTC_NO_MATCH &&  !offset) // Gotcha
            {
                state=IVTC_PROCESSING;
                mode=match;
                offsetInSequence=1;
                startSequence=incomingNum;
                aprintf("Synced mode = %d\n",mode);
            }
        }
            break;
        case IVTC_PROCESSING:
        {
            int left,right;
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
            if(mode==IVTC_LEFT_MATCH)
            {
                int med=left;
                left=right;
                right=med;
            }else
            {
               
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
            copyField(image,source1,true);
            copyField(image,source1,false);
                     
            
            offsetInSequence++;
            if(offsetInSequence>=PERIOD)
                state=IVTC_SYNCING;
             vidCache->unlockAll();
            *fn=currentNum;
            currentNum++,
            incomingNum++;
            
            
            return true;               
            
        }
            break;
    }
    
    ADMImage *i=vidCache->getImage(incomingNum);
    if(!i)
    {
        vidCache->unlockAll();
        return false;
    }
    image->duplicateFull(i);
    vidCache->unlockAll();
    *fn=currentNum;
    currentNum++,
    incomingNum++;
    return true;               
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         admIvtc::getCoupledConf(CONFcouple **couples)
{

    return ADM_paramSave(couples, dupeRemover_param,&configuration);
}

void admIvtc::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, dupeRemover_param, &configuration);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *admIvtc::getConfiguration(void)
{
    static char bfer[1024];

    sprintf(bfer,"IVTC Dupe Removed : Threshold =%u",(unsigned int)configuration.threshold);
    return bfer;
}
/**
 *
 * @param usSeek
 * @return
 */
bool         admIvtc::goToTime(uint64_t usSeek)
{
    vidCache->flush();
    incomingNum=0;
    currentNum=0;
    state=IVTC_SYNCING;
    return previousFilter->goToTime(usSeek);
}
/**
    \fn configure
*/
bool admIvtc::configure( void)
{

#define PX(x) &(configuration.x)
        diaElemUInteger   threshold(PX(threshold),QT_TRANSLATE_NOOP("ivtcRemover","_Noise:"),0,255);
        diaElemToggle     show(PX(show),QT_TRANSLATE_NOOP("ivtcRemover","_Show:"));


        diaMenuEntry menuMode[]={
                {0,      QT_TRANSLATE_NOOP("ivtcRemover","Full")},
                {1,      QT_TRANSLATE_NOOP("ivtcRemover","Fast")},
                {2,      QT_TRANSLATE_NOOP("ivtcRemover","VeryFast")}
                };



        diaElemMenu      eMode(&configuration.mode,QT_TRANSLATE_NOOP("ivtcRemover","_Frame rate change:"),3,menuMode);

        diaElem *elems[3]={&threshold,&show,&eMode};
        return diaFactoryRun(QT_TRANSLATE_NOOP("ivtcRemover","DupeRemover"),3,elems);
}
/**
*/
static uint32_t smallDiff(uint8_t  *s1,uint8_t *s2,uint32_t noise, int count)
{
uint32_t df=0;
uint32_t delta;
    for(int x=0;x<count;x++)
    {
            delta=abs((int)(s1[x])-(int)(s2[x]));
            if(delta>noise)
                    df+=delta;
    }
    return df;
}

/**

*/
/* 3000 * 3000 max size, using uint32_t is safe... */
static uint32_t computeFieldDiff(uint8_t  *s1,uint8_t *s2,uint32_t noise,int w,int h, int stride1, int stride2)
{
uint32_t df=0;
uint32_t delta;

    for(int y=0;y<h-1;y+=2)
    {
        for(int x=0;x<w;x++)
        {
                delta=abs((int)(s1[x])-(int)(s2[x]));
                if(delta>noise)
                        df+=delta;
        }
        s1+=stride1*2;
        s2+=stride2*2;
    }
    return df;
}
/**
 * \fn lumaDiff
*/
uint32_t admIvtc::lumaDiff(bool field,ADMImage *src1,ADMImage *src2,uint32_t noise)
{
    int stride1=src1->GetPitch(PLANAR_Y);
    int stride2=src2->GetPitch(PLANAR_Y);
    uint8_t *p1=YPLANE(src1);
    uint8_t *p2=YPLANE(src2);
    int w=src1->GetWidth(PLANAR_Y);
    int h=src1->GetHeight(PLANAR_Y);
    if(field)
    {
        p1+=stride1;
        p2+=stride2;
    }
            
    return computeFieldDiff(p1,p2,
                noise,
                w,h,
                stride1,stride2);
}

/**
 * \fn computeDelta
 * @param left
 * @param right
 * @param threshold
 * @return
 */
ivtcMatch admIvtc::computeMatch(ADMImage *left,ADMImage *right, int threshold)
{
   
        uint32_t even=lumaDiff(false,left,right,threshold);
        uint32_t odd= lumaDiff(true,left,right,threshold);
        
#define MATCH_THRESHOLD        
        
        ivtcMatch match=IVTC_NO_MATCH;
        const char *r="-";
        if(even > 10*odd)
        {
           r="Left match";
           match= IVTC_LEFT_MATCH;
        }
        if(odd > 10*even) 
        {
            r="Right match";
            match= IVTC_RIGHT_MATCH;
        }
        aprintf("[Even:%d : Odd:%d] %s\n",even,odd,r);

        return match;
}
/************************************************/
//EOF
