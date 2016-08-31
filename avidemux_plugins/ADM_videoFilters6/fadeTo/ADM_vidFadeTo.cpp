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
#include "ADM_vidMisc.h"
#include "fade.h"
#include "fade_desc.cpp"
/**
        \class AVDM_FadeTo
 *      \brief fade video plugin
 */
class AVDM_FadeTo : public  ADM_coreVideoFilterCached
{
protected:
                fade            param;
                bool            buildLut(void);
                ADMImage        *first;
                bool            process(ADMImage *source,ADMImage *source2, ADMImage *dest,int offset);
public:
                                AVDM_FadeTo(ADM_coreVideoFilter *previous,CONFcouple *conf);
                                ~AVDM_FadeTo();

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



DECLARE_VIDEO_FILTER(AVDM_FadeTo,
                1,0,0,              // Version
                     ADM_UI_ALL,         // UI
                     VF_TRANSFORM,            // Category
                     "fadeTo",            // internal name (must be uniq!)
                     QT_TRANSLATE_NOOP("fadeTo","Fade"),            // Display name
                     QT_TRANSLATE_NOOP("fadeTo","Fade.") // Description
                 );   
/**
 * \fn configure
 * \brief UI configuration
 * @param 
 * @return 
 */
bool  AVDM_FadeTo::configure()
{
    
  uint32_t start,end;
  
  
while(1)
{
    
    uint32_t mx=9*3600*1000;
  
    diaElemTimeStamp start(&(param.startFade),QT_TRANSLATE_NOOP("fadeTo","_Start time (ms):"),0,mx);
    diaElemTimeStamp end(&(param.endFade),QT_TRANSLATE_NOOP("fadeTo","_End time (ms):"),0,mx);
    diaElem *elems[2]={&start,&end};
  
    if( diaFactoryRun(QT_TRANSLATE_NOOP("fadeTo","Fade"),2+0*1,elems))
    {
        
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
const char   *AVDM_FadeTo::getConfiguration(void)
{
    static char conf[256];
    std::string start=std::string(ADM_us2plain(param.startFade*1000LL));
    std::string end=std::string(ADM_us2plain(param.endFade*1000LL));
    snprintf(conf,255," Fade : Start %s End %s",start.c_str(),end.c_str());
    return conf;
}

/**
 * \fn ctor
 * @param in
 * @param couples
 */
AVDM_FadeTo::AVDM_FadeTo(ADM_coreVideoFilter *in,CONFcouple *setup) :  ADM_coreVideoFilterCached(3,in,setup)
{
    if(!setup || !ADM_paramLoad(setup,fade_param,&param))
    {
        // Default value
        param.startFade=0; 
        param.endFade=0;
    }
    buildLut();
    nextFrame=0;
    first=NULL;
}
/**
 * \fn setCoupledConf
 * \brief save current setup from couples
 * @param couples
 */
void AVDM_FadeTo::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, fade_param, &param);
}

/**
 * \fn getCoupledConf
 * @param couples
 * @return setup as couples
 */
bool         AVDM_FadeTo::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, fade_param,&param);
}

/**
 * \fn dtor
 */
AVDM_FadeTo::~AVDM_FadeTo(void)
{
    if(first)      
    {
      delete first;
      first=NULL;
    }
}
/**
 * 
 * @param source
 * @param source2
 * @param dest
 * @param offset
 * @return 
 */
bool AVDM_FadeTo::process(ADMImage *source,ADMImage *source2, ADMImage *dest,int offset)
{
  
    uint8_t *splanes[3],*splanes2[3],*dplanes[3];
    int      spitches[3],spitches2[3],dpitches[3];

    source->GetReadPlanes(splanes);
    source->GetPitches(spitches);
    source2->GetReadPlanes(splanes2);
    source2->GetPitches(spitches2);
    dest->GetWritePlanes(dplanes);
    dest->GetPitches(dpitches);
    

    for(int i=0;i<3;i++)
    {
        uint16_t *indx=lookupChroma[offset];
        uint16_t *revindex=lookupChroma[255-offset];
        int colorOffset=128<<8;
        if(!i) 
        {
          indx=lookupLuma[offset];
          revindex=lookupLuma[255-offset];
          colorOffset=0;
        }
        int    w=(int)dest->GetWidth((ADM_PLANE)i);
        int    h=(int)dest->GetHeight((ADM_PLANE)i);
        uint8_t *s=splanes[i];
        uint8_t *s2=splanes2[i];
        uint8_t *d=dplanes[i];
        for(int y=0;y<h;y++)
        {
            for(int x=0;x<w;x++)
            {
                int value=s[x];
                int value2=s2[x];
                d[x]=(indx[value]+revindex[value2]-colorOffset)>>8;
            }
            d+=dpitches[i];
            s+=spitches[i];
            s2+=spitches2[i];
        }        
    }
    return true;
}

/**
 * \fn getNextFrame
 * @param fn
 * @param image
 * @return 
 */
bool AVDM_FadeTo::getNextFrame(uint32_t *fn,ADMImage *image)
{
  *fn=nextFrame;
  ADMImage *next= vidCache->getImage(nextFrame);
  if(!next)
  {
      ADM_info("[Fade] Cant get image \n");
      return false;
  }
  
  image->Pts=next->Pts;
  
  uint64_t absPts=next->Pts+getAbsoluteStartTime();
  
  bool out_of_scope=false;
  if(absPts<param.startFade*1000LL) out_of_scope=true;
  if(absPts>param.endFade*1000LL)   out_of_scope=true;
  
  
  if(!out_of_scope && !first)
  {
      first=new ADMImageDefault(next->GetWidth (PLANAR_Y),next->GetHeight(PLANAR_Y));
      first->duplicateFull (next);
  }
  if( out_of_scope || !first)
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
        in=absPts-param.startFade*1000LL;
    }
    in=in/scope;
    in*=255;
  
  
  uint32_t offset=(uint32_t)floor(in+0.4); // normalized to 0--255, begin -- end
  
  
    process(first,next,image,offset);
  
    vidCache->unlockAll();
    nextFrame++;
    return 1;
}
/**
 * \fn buildLut
 * \brief compute the Lut we will be using
 * @return 
 */
bool  AVDM_FadeTo::buildLut(void)
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



