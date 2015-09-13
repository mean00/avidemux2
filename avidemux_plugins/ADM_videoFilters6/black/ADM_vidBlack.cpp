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
#include "black.h"
#include "black_desc.cpp"

/**
        \class AVDM_black
 *      \brief fade video plugin
 */
class AVDM_black : public  ADM_coreVideoFilterCached
{
protected:
                black            param;
public:
                             AVDM_black(ADM_coreVideoFilter *previous,CONFcouple *conf);
                             ~AVDM_black();

        virtual const char  *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;           /// Start graphical user interface
};

// Add the hook to make it valid plugin



DECLARE_VIDEO_FILTER(AVDM_black,
                     1,0,0,              // Version
                     ADM_UI_ALL,         // UI
                     VF_TRANSFORM,            // Category
                     "black",            // internal name (must be uniq!)
                     QT_TRANSLATE_NOOP("black","Black"),            // Display name
                     QT_TRANSLATE_NOOP("black","Replace a section by black.") // Description
                 );   
/**
 * \fn configure
 * \brief UI configuration
 * @param 
 * @return 
 */
bool  AVDM_black::configure()
{
    
        uint32_t mx=9*3600*1000;
        diaElemTimeStamp start(&(param.startBlack),QT_TRANSLATE_NOOP("fade","_Start time (ms):"),0,mx);
        diaElemTimeStamp end(&(param.endBlack),QT_TRANSLATE_NOOP("fade","_End time (ms):"),0,mx);
        diaElem *elems[2]={&start,&end};
        return diaFactoryRun(QT_TRANSLATE_NOOP("black","Replace by Black"),2+0*1,elems);
}
/**
 *      \fn getConfiguration
 * 
 */
const char   *AVDM_black::getConfiguration(void)
{
        static char conf[1024];
        std::string startTime=std::string(ADM_us2plain(param.startBlack*1000));
        std::string endTime=std::string(ADM_us2plain(param.endBlack*1000));
	snprintf(conf,255," Black : Start %s End %s",startTime.c_str(),endTime.c_str());
        return conf;
}

/**
 * \fn ctor
 * @param in
 * @param couples
 */
AVDM_black::AVDM_black(ADM_coreVideoFilter *in,CONFcouple *setup) :  ADM_coreVideoFilterCached(3,in,setup)
{
    if(!setup || !ADM_paramLoad(setup,black_param,&param))
    {
        // Default value
        param.startBlack=0; 
        param.endBlack=0;
    }
    nextFrame=0;
}
/**
 * \fn setCoupledConf
 * \brief save current setup from couples
 * @param couples
 */
void AVDM_black::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, black_param, &param);
}

/**
 * \fn getCoupledConf
 * @param couples
 * @return setup as couples
 */
bool         AVDM_black::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, black_param,&param);
}

/**
 * \fn dtor
 */
AVDM_black::~AVDM_black(void)
{
                
  
}

/**
 * \fn getNextFrame
 * @param fn
 * @param image
 * @return 
 */
bool AVDM_black::getNextFrame(uint32_t *fn,ADMImage *image)
{
  *fn=nextFrame;
  ADMImage *next= vidCache->getImage(nextFrame);
  if(!next)
  {
      ADM_info("[Fade] Cant get image \n");
      return false;
  }
  
  image->copyInfo(next);  
  bool out_of_scope=false;
  
  if(next->Pts<param.startBlack*1000LL) out_of_scope=true;
  if(next->Pts>=param.endBlack*1000LL)   out_of_scope=true;
  
  if( out_of_scope)
  {
      image->duplicate(next);
      nextFrame++;
      vidCache->unlockAll();
      return true;
  }
  
  vidCache->unlockAll();
  nextFrame++;
  image->blacken();
  return true;

}
//EOF



