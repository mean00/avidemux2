/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>

#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "DIA_coreToolkit.h"
#include "DIA_factory.h"
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
                bool            buildLut(void);
public:
                                AVDM_Fade(ADM_coreVideoFilter *previous,CONFcouple *conf);
                                ~AVDM_Fade();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;           /// Start graphical user interface
        uint16_t             lookupLuma[256][256];
        uint16_t             lookupChroma[256][256];

};

// Add the hook to make it valid plugin



DECLARE_VIDEO_FILTER(AVDM_Fade,
                1,0,0,              // Version
                     ADM_UI_ALL,         // UI
                     VF_TRANSFORM,            // Category
                     "fade",            // internal name (must be uniq!)
                     "Fade",            // Display name
                     "Fade in/out." // Description
                 );   
/**
 * \fn configure
 * \brief UI configuration
 * @param 
 * @return 
 */
bool  AVDM_Fade::configure()
{
  
  diaMenuEntry menuE[2]={{0,QT_TR_NOOP("Out"),QT_TR_NOOP("Fade out")},{1,QT_TR_NOOP("In"),QT_TR_NOOP("Fade in")}};
  uint32_t start,end;
  
  
while(1)
{
    uint32_t eInOut=(uint32_t)param.inOut;
    
    uint32_t mx=9*3600*1000;
    diaElemMenu     menu(&(eInOut),QT_TR_NOOP("_Fade type:"), 2,menuE);
    diaElemTimeStamp start(&(param.startFade),QT_TR_NOOP("_Start time (ms):"),0,mx);
    diaElemTimeStamp end(&(param.endFade),QT_TR_NOOP("_End time (ms):"),0,mx);
    diaElem *elems[3]={&menu,&start,&end};
  
    if( diaFactoryRun(QT_TR_NOOP("Fade to black"),3+0*1,elems))
    {
        
        param.inOut=eInOut;
        buildLut();
        return 1;
    }else
        return 0;
} 
  return 1;
}
/**
 *      \fn getConfiguration
 * 
 */
const char   *AVDM_Fade::getConfiguration(void)
{
        static char conf[256];
	snprintf(conf,255," Fade : Start %u End %u",param.startFade,param.endFade);
        return conf;
}

/**
 * \fn ctor
 * @param in
 * @param couples
 */
AVDM_Fade::AVDM_Fade(ADM_coreVideoFilter *in,CONFcouple *setup) :  ADM_coreVideoFilterCached(3,in,setup)
{
    if(!setup || !ADM_paramLoad(setup,fade_param,&param))
    {
        // Default value
        param.startFade=0; 
        param.endFade=0;
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
  
  bool out_of_scope=false;
  if(next->Pts<param.startFade*1000LL) out_of_scope=true;
  if(next->Pts>param.endFade*1000LL)   out_of_scope=true;
  
  
  if( out_of_scope)
  {
      image->duplicate(next);
      nextFrame++;
      vidCache->unlockAll();
      return true;
  }
  
  double scope=1000LL*(param.endFade-param.startFade);
  double in;
  if(!scope) 
  {
      scope=1;
      in=1;
  }else
  {
        in=next->Pts-param.startFade*1000LL;
  }
    in=in/scope;
    in*=255;
  
  
  uint32_t offset=(uint32_t)floor(in+0.4); // normalized to 0--255
  

  if(param.toBlack)
  {
    uint8_t *splanes[3],*dplanes[3];
    uint32_t spitches[3],dpitches[3];

    next->GetReadPlanes(splanes);
    next->GetPitches(spitches);
    image->GetReadPlanes(dplanes);
    image->GetPitches(dpitches);

    for(int i=0;i<3;i++)
    {
        uint16_t *indx=lookupChroma[offset];
        if(!i) indx=lookupLuma[offset];
        int    w=(int)image->GetWidth((ADM_PLANE)i);
        int    h=(int)image->GetHeight((ADM_PLANE)i);
        uint8_t *s=splanes[i];
        uint8_t *d=dplanes[i];
        for(int y=0;y<h;y++)
        {
            for(int x=0;x<w;x++)
            {
                d[x]=indx[s[x]]>>8;
            }
            d+=dpitches[i];
            s+=dpitches[i];
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
    if(!param.inOut) ration=255-i;
    else ration=i;
    for(int r=0;r<256;r++)
    {
      f=r;
      f=f*ration;
      lookupLuma[i][r]=(uint16_t)(f+0.4);

      f=r-128;
      f=f*ration;
      lookupChroma[i][r]=(128<<8)+(uint16_t)(f+0.4);

    }
    
  }
  return true;
}
//EOF



