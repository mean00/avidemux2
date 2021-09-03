/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>
#include <string>
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "DIA_coreToolkit.h"
#include "DIA_factory.h"
#include <ADM_vidMisc.h>

#include "fade.h"
#include "fade_desc.cpp"
/**
        \class AVDM_Fade
 *      \brief fade video plugin
 */
class AVDM_Fade : public  ADM_coreVideoFilterCached
{
protected:
                fade            param;
                uint32_t        max;
                void            boundsCheck(void);
                bool            buildLut(void);

public:
                                AVDM_Fade(ADM_coreVideoFilter *previous,CONFcouple *conf);
                                ~AVDM_Fade();

        virtual const char      *getConfiguration(void); /// Return  current configuration as a human readable string
        virtual bool            getNextFrame(uint32_t *fn,ADMImage *image); /// Return the next image
        virtual bool            getCoupledConf(CONFcouple **couples) ; /// Return the current filter configuration
        virtual void            setCoupledConf(CONFcouple *couples);
        virtual bool            configure(void) ; /// Start graphical user interface
        uint16_t                lookupLuma[256][256];
        uint16_t                lookupChroma[256][256];

};

// Add the hook to make it valid plugin



DECLARE_VIDEO_FILTER(AVDM_Fade,
                    1,0,0,          // Version
                    ADM_UI_ALL,     // UI
                    VF_TRANSITION,   // Category
                    "fadeToBlack",  // internal name (must be uniq!)
                    QT_TRANSLATE_NOOP("fadeToBlack","Fade to black"), // Display name
                    QT_TRANSLATE_NOOP("fadeToBlack","Fade to black in/out.") // Description
);
/**
 * \fn configure
 * \brief UI configuration
 * @param 
 * @return 
 */
bool  AVDM_Fade::configure()
{
    diaMenuEntry menuE[2]={{0,QT_TRANSLATE_NOOP("fadeToBlack","Out"),QT_TRANSLATE_NOOP("fadeToBlack","Fade out")},{1,QT_TRANSLATE_NOOP("fadeToBlack","In"),QT_TRANSLATE_NOOP("fadeToBlack","Fade in")}};

    uint32_t eInOut=(uint32_t)param.inOut;

    diaElemMenu     menu(&(eInOut),QT_TRANSLATE_NOOP("fadeToBlack","_Fade type:"), 2,menuE);
    diaElemTimeStamp start(&(param.startFade),QT_TRANSLATE_NOOP("fadeToBlack","_Start time:"),0,max);
    diaElemTimeStamp end(&(param.endFade),QT_TRANSLATE_NOOP("fadeToBlack","_End time:"),0,max);
    diaElem *elems[3]={&menu,&start,&end};

    if( diaFactoryRun(QT_TRANSLATE_NOOP("fadeToBlack","Fade to black"),3+0*1,elems))
    {
        param.inOut=eInOut;
        buildLut();
        boundsCheck();
        return 1;
    }
    return 0;
}
/**
 *      \fn getConfiguration
 * 
 */
const char   *AVDM_Fade::getConfiguration(void)
{
 static char conf[256];
    std::string start=std::string(ADM_us2plain(param.startFade*1000LL));
    std::string end=std::string(ADM_us2plain(param.endFade*1000LL));
    const char *type=(param.inOut)? "in" : "out";
    snprintf(conf,255," Fade %s: Start %s End %s",type,start.c_str(),end.c_str());
    return conf;
}

/**
 * \fn ctor
 * @param in
 * @param couples
 */
AVDM_Fade::AVDM_Fade(ADM_coreVideoFilter *in,CONFcouple *setup) :  ADM_coreVideoFilterCached(3,in,setup)
{
    max=(uint32_t)(in->getInfo()->totalDuration/1000);
    if(!setup || !ADM_paramLoad(setup,fade_param,&param))
    {
        // Default value
        param.startFade=info.markerA / 1000LL;
        param.endFade=info.markerB / 1000LL;
        param.inOut=0;
        param.toBlack=true;

    }
    buildLut();
    nextFrame=0;
}
/**
 * \fn setCoupledConf
 * \brief save current setup from couples
 * @param couples
 */
void AVDM_Fade::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, fade_param, &param);
}

/**
 * \fn getCoupledConf
 * @param couples
 * @return setup as couples
 */
bool         AVDM_Fade::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, fade_param,&param);
}

/**
 * \fn dtor
 */
AVDM_Fade::~AVDM_Fade(void)
{
}

/**
 * \fn getNextFrame
 * @param fn
 * @param image
 * @return 
 */
bool AVDM_Fade::getNextFrame(uint32_t *fn,ADMImage *image)
{
    *fn=nextFrame;
    ADMImage *next= vidCache->getImage(nextFrame);
    if(!next)
    {
        ADM_info("[Fade] Cant get image \n");
        return false;
    }
    image->Pts=next->Pts;
    uint32_t absPtsMs=(next->Pts+getAbsoluteStartTime())/1000LL;
    bool out_of_scope=false;

    if(absPtsMs < param.startFade) out_of_scope=true;
    if(absPtsMs >= param.endFade)   out_of_scope=true;

    if( out_of_scope)
    {
        image->duplicate(next);
        nextFrame++;
        vidCache->unlockAll();
        return true;
    }
    double scope=(param.endFade-param.startFade);
    double in;
    if(!scope)
    {
        scope=1;
        in=1;
    }else
    {
        in=absPtsMs-param.startFade;
    }
    in=in/scope;
    in*=255;

    uint32_t offset=(uint32_t)floor(in+0.4); // normalized to 0--255

    if(param.toBlack)
    {
        uint8_t *splanes[3],*dplanes[3];
        int spitches[3],dpitches[3];

        next->GetReadPlanes(splanes);
        next->GetPitches(spitches);
        image->GetReadPlanes(dplanes);
        image->GetPitches(dpitches);

        for(int i=0;i<3;i++)
        {
            uint16_t *indx=lookupChroma[offset];
            if(!i) indx=lookupLuma[offset];
            int w=(int)image->GetWidth((ADM_PLANE)i);
            int h=(int)image->GetHeight((ADM_PLANE)i);
            uint8_t *s=splanes[i];
            uint8_t *d=dplanes[i];
            for(int y=0;y<h;y++)
            {
                for(int x=0;x<w;x++)
                {
                    d[x]=indx[s[x]]>>8;
                }
                d+=dpitches[i];
                s+=spitches[i];
            }
        }
        vidCache->unlockAll();
        nextFrame++;
        return 1;
    }
    vidCache->unlockAll();
    nextFrame++;
    return 1;
}
/**
 * \fn buildLut
 * \brief compute the Lut we will be using
 * @return 
 */
bool  AVDM_Fade::buildLut(void)
{
    float f,ration;
    for(int i=0;i<256;i++)
    {
        if(!param.inOut) ration=255.-i;
        else ration=(float)i;
        for(int r=0;r<256;r++)
        {
            f=(float)r;
            f=f*ration;
            lookupLuma[i][r]=(uint16_t)(f+0.4);

            f=r-128.;
            f=f*ration;
            f+=(float)(128<<8);
            lookupChroma[i][r]=(uint16_t)f;
        }
    }
    return true;
}
/**
 * \fn boundsCheck
 * \brief Reset invalid start and end values
 */
void AVDM_Fade::boundsCheck(void)
{
    if(param.endFade < param.startFade)
    {
        uint32_t tmp=param.startFade;
        param.startFade=param.endFade;
        param.endFade=tmp;
    }
    if(param.endFade > max)
        param.endFade=max;
    if(param.startFade > max)
        param.startFade=0;
}
//EOF
