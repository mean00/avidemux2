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
        configuration.removeDupe=false;

    }
    for(int i=0;i<2;i++)
        spare[i]=new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
    myName="admIvtc";



    startSequence=0;
    state=IVTC_SYNCING;
}
/**
    \fn admIvtc
    \brief destructor
*/
admIvtc::~admIvtc()
{
    for(int i=0;i<2;i++)
    {
        delete spare[i];
        spare[i]=NULL;
    }
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
 bool copyField(ADMImage *target, ADMImage *source, bool top)
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
    nextFrame=0;
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
        diaElemToggle     remove(PX(removeDupe),QT_TRANSLATE_NOOP("ivtcRemover","_Remove duplicate:"));


        diaMenuEntry menuMode[]={
                {0,      QT_TRANSLATE_NOOP("ivtcRemover","Full")},
                {1,      QT_TRANSLATE_NOOP("ivtcRemover","Fast")},
                {2,      QT_TRANSLATE_NOOP("ivtcRemover","VeryFast")}
                };



        diaElemMenu      eMode(&configuration.mode,QT_TRANSLATE_NOOP("ivtcRemover","_Frame rate change:"),3,menuMode);

        diaElem *elems[4]={&threshold,&remove,&eMode,&show};
        return diaFactoryRun(QT_TRANSLATE_NOOP("ivtcRemover","DupeRemover"),4,elems);
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
uint32_t admIvtc::lumaDiff(bool bottom,ADMImage *src1,ADMImage *src2,uint32_t noise)
{
    int stride1=src1->GetPitch(PLANAR_Y);
    int stride2=src2->GetPitch(PLANAR_Y);
    uint8_t *p1=YPLANE(src1);
    uint8_t *p2=YPLANE(src2);
    int w=src1->GetWidth(PLANAR_Y);
    int h=src1->GetHeight(PLANAR_Y);
    if(bottom)
    {
        p1+=stride1;
        p2+=stride2;
    }
            
    return computeFieldDiff(p1,p2,
                noise,
                w,h>>configuration.mode,
                stride1<<configuration.mode,stride2<<configuration.mode);
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
   
        uint32_t top=lumaDiff(false,left,right,threshold);
        uint32_t bottom= lumaDiff(true,left,right,threshold);
        
#define MATCH_THRESHOLD        
        
        ivtcMatch match=IVTC_NO_MATCH;
        const char *r="-";
        if(top > 10*bottom)
        {
           r="BOTTOM match";
           match= IVTC_BOTTOM_MATCH;
        }
        if(bottom > 10*top) 
        {
            r="Right match";
            match= IVTC_TOP_MATCH;
        }
        aprintf("[Even:%d : Odd:%d] %s\n",top,bottom,r);

        return match;
}
bool            admIvtc::displayStatus(ADMImage *image,const char *st)
{
    if(configuration.show)
          image->printString(2,16,st);
    return true;
}


/************************************************/
//EOF
