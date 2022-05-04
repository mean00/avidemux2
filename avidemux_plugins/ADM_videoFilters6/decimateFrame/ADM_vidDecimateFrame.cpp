/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cmath>
#include <string>
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "DIA_coreToolkit.h"
#include "DIA_factory.h"
#include "ADM_vidMisc.h"
#include "decimateFrame.h"
#include "decimateFrame_desc.cpp"
/**
        \class AVDM_DecimateFrame
 */
class AVDM_DecimateFrame : public  ADM_coreVideoFilter
{
protected:
                decimateFrame   param;
                int             count;
                uint64_t        pts;
                unsigned int    cIndex;
                ADMImage        *carousel[4];

public:
                                AVDM_DecimateFrame(ADM_coreVideoFilter *previous,CONFcouple *conf);
                                ~AVDM_DecimateFrame();

                double          diff(ADMImage * a, ADMImage * b);
        virtual const char      *getConfiguration(void); /// Return  current configuration as a human readable string
        virtual bool            getNextFrame(uint32_t *fn,ADMImage *image); /// Return the next image
        virtual bool            getCoupledConf(CONFcouple **couples); /// Return the current filter configuration
        virtual void            setCoupledConf(CONFcouple *couples);
        virtual bool            configure(void); /// Start graphical user interface
        virtual bool            goToTime(uint64_t time);

};

// Add the hook to make it valid plugin



DECLARE_VIDEO_FILTER(AVDM_DecimateFrame,
                    1,0,0,          // Version
                    ADM_UI_ALL,     // UI
                    VF_TRANSFORM,   // Category
                    "decimateFrame",       // internal name (must be uniq!)
                    QT_TRANSLATE_NOOP("decimateFrame","Decimate"), // Display name
                    QT_TRANSLATE_NOOP("decimateFrame","Drop duplicate frames.") // Description
);
/**
 * \fn configure
 * \brief UI configuration
 * @param 
 * @return 
 */
bool  AVDM_DecimateFrame::configure()
{
    diaElemInteger  eThreshold(&param.threshold,QT_TRANSLATE_NOOP("decimateFrame","Duplicate threshold:"),0,9999);
    diaElemToggle eLimitDropCount(&param.limitDropCount,QT_TRANSLATE_NOOP("decimateFrame","Limit drop count"));
    diaElemInteger  eMaxDropCount(&param.maxDropCount,QT_TRANSLATE_NOOP("decimateFrame","Consecutive frame drop limit:"),1,100);
    eLimitDropCount.link(1, &eMaxDropCount);
    diaElemToggle eEvaluate(&param.evaluate,QT_TRANSLATE_NOOP("decimateFrame","Evaluation mode (no drop + print difference info)"));
    diaElemReadOnlyText eNote(NULL,QT_TRANSLATE_NOOP("decimateFrame","Note: this filter won't change the reported frame rate"),NULL);


    diaElem *elems[5]={&eThreshold,&eLimitDropCount,&eMaxDropCount,&eEvaluate,&eNote};

    if( diaFactoryRun(QT_TRANSLATE_NOOP("decimateFrame","Decimate"),5,elems))
    {
        return 1;
    }
    return 0;
}
/**
 *      \fn getConfiguration
 * 
 */
const char   *AVDM_DecimateFrame::getConfiguration(void)
{
    static char conf[256];
    if (param.evaluate)
    {
        snprintf(conf,255,"-=EVALUATION MODE=-");
    }
    else
    {
        if (param.limitDropCount)
        {
            snprintf(conf,255,"Threshold: %d. Consecutive frame drop limit: %d", param.threshold, param.maxDropCount);
        }
        else
        {
            snprintf(conf,255,"Threshold: %d. No frame drop limit", param.threshold);
        }
    }
    return conf;
}

/**
 * \fn ctor
 * @param in
 * @param couples
 */
AVDM_DecimateFrame::AVDM_DecimateFrame(ADM_coreVideoFilter *in,CONFcouple *setup) :  ADM_coreVideoFilter(in,setup)
{
    if(!setup || !ADM_paramLoad(setup,decimateFrame_param,&param))
    {
        // Default value
        param.limitDropCount = false;
        param.maxDropCount = 1;
        param.threshold = 100;
        param.evaluate = false;
        
    }
    nextFrame=0;
    for (int i=0; i<4; i++)
        carousel[i] = new ADMImageDefault(info.width,info.height);
    pts=ADM_NO_PTS;
    cIndex=0;
    count=0;
}
/**
 * \fn setCoupledConf
 * \brief save current setup from couples
 * @param couples
 */
void AVDM_DecimateFrame::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, decimateFrame_param, &param);
}

/**
 * \fn getCoupledConf
 * @param couples
 * @return setup as couples
 */
bool         AVDM_DecimateFrame::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, decimateFrame_param,&param);
}

/**
 * \fn dtor
 */
AVDM_DecimateFrame::~AVDM_DecimateFrame(void)
{
    for (int i=0; i<4; i++)
        delete carousel[i];
}
/**
 * \fn goToTime
 */
bool AVDM_DecimateFrame::goToTime(uint64_t time)
{
    count=0;
    cIndex=0;
    return ADM_coreVideoFilter::goToTime(time);
}

/**
 * \fn diff
 */
double AVDM_DecimateFrame::diff(ADMImage * a, ADMImage * b)
{
    int width=a->GetWidth(PLANAR_Y); 
    int height=a->GetHeight(PLANAR_Y);
    int stride;
    uint8_t * ptr1, * ptr2;
    stride=a->GetPitch(PLANAR_Y);
    ptr1=a->GetReadPtr(PLANAR_Y);    
    ptr2=b->GetReadPtr(PLANAR_Y);

    long int p,q;
    long int sum, max;
    max = 0;
    for(int y=0;y<height;y++)
    {
        sum = 0;
        for (int x=0;x<width;x++)
        {
            p = ptr1[x];
            q = ptr2[x];
            sum += (p-q)*(p-q);
        }
        
        if (sum > max)
        {
            max = sum;
        }

        ptr1+=stride;
        ptr2+=stride;

    }
    return (double)max/(double)width;    
}
/**
 * \fn getNextFrame
 * @param fn
 * @param image
 * @return 
 */
bool AVDM_DecimateFrame::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if (param.evaluate)
    {
        char info[256];
        if(!previousFilter->getNextFrame(fn, image)) return false;
        if (!count)
        {
            count++;
            snprintf(info,256,"Frame diff: not available for the first frame");
        }
        else
        {
            snprintf(info,256,"Frame diff: %9.03f", diff(image, carousel[0]));
        }
        carousel[0]->duplicateFull(image);
        image->printString(0,1,info);
        snprintf(info,256,"Current threshold: %d", param.threshold);
        image->printString(0,2,info);
        return true;
    }

    uint32_t fnd;
    double difference;
    while(1)
    {
        cIndex++;
        if(!previousFilter->getNextFrame(&fnd,carousel[cIndex & 3]))
        {
            if (!count)
            {
                return false;
            }
            else
            {
                cIndex--;
                image->duplicateFull (carousel[cIndex & 3]);
                image->Pts=pts;
                count = 0;
                *fn=nextFrame++;
                return true;
            }
        }

        if(!count)
        {
            pts=carousel[cIndex & 3]->Pts;
            count += 1;
            continue;
        }
        
        difference = diff(carousel[cIndex & 3], carousel[(cIndex-1) & 3]);

        if (difference < param.threshold)
        {
            count += 1;
            if ((!param.limitDropCount) || (count <= param.maxDropCount))
                continue;
        }

        count = 1;
        image->duplicateFull (carousel[(cIndex-1) & 3]);
        image->Pts=pts;
    
        pts = carousel[cIndex & 3]->Pts;
        *fn=nextFrame++;
        return true;
    }
}

//EOF
