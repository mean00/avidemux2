/***************************************************************************
    \file ADM_vidchangeFps
    \author mean fixounet@free.fr
    \brief Simple filter that rescales timing from oldFps to newFps
 ***************************************************************************/

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

#include "confChangeFps.h"
#include "confChangeFps_desc.cpp"

#if 1
    #define aprintf(...) {}
#else
    #define aprintf ADM_info
#endif

typedef struct 
{
    const char *desc;
    uint32_t num;
    uint32_t den;
}PredefinedFps_t;

const PredefinedFps_t predefinedFps[]=
{
 {QT_TRANSLATE_NOOP("changeFps","Custom"),         10000,1000},
 {QT_TRANSLATE_NOOP("changeFps","25  (PAL)"),      25000,1000},
 {QT_TRANSLATE_NOOP("changeFps","23.976 (Film)"),  24000,1001},
 {QT_TRANSLATE_NOOP("changeFps","29.97 (NTSC)"),   30000,1001},
 {QT_TRANSLATE_NOOP("changeFps","50 (Pal)"),       50000,1000},
 {QT_TRANSLATE_NOOP("changeFps","59.93  (NTSC)"),  60000,1001}
};

#define nbPredefined (sizeof(predefinedFps)/sizeof(PredefinedFps_t))

/**
    \class changeFps

*/
class  changeFps:public ADM_coreVideoFilter
{
protected:
        confChangeFps       configuration;
        bool                updateTimingInfo(void);
public:
                            changeFps(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~changeFps();
        bool                goToTime(uint64_t usSeek);
        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;           /// Start graphical user interface
};
//***********************************
// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   changeFps,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_TRANSFORM,            // Category
                        "changeFps",            // internal name (must be uniq!)
                        "Change FPS",            // Display name
                        "Speed up/slow down the video as if altering fps. This filter changes duration." // Description
                    );

/**
    \fn getConfiguration
*/
const char *changeFps::getConfiguration( void )
{
static char buf[100];
 snprintf(buf,99," Resample from %2.2f to %2.2f fps",
        (double)configuration.oldFpsNum/configuration.oldFpsDen,
        (double)configuration.newFpsNum/configuration.newFpsDen);
 return buf;  
}
/**
    \fn ctor
*/
changeFps::changeFps(  ADM_coreVideoFilter *previous,CONFcouple *setup) : ADM_coreVideoFilter(previous,setup)
{
    if(!setup || !ADM_paramLoad(setup,confChangeFps_param,&configuration))
    {
        // Default value
        configuration.newMode=0;
        configuration.newFpsNum=ADM_Fps1000FromUs(previous->getInfo()->frameIncrement);
        configuration.newFpsDen=1000;
        configuration.oldMode=0;
        configuration.oldFpsNum=ADM_Fps1000FromUs(previous->getInfo()->frameIncrement);
        configuration.oldFpsDen=1000;

    }
    updateTimingInfo();
}
/**
    \fn dtor

*/
changeFps::~changeFps()
{
}

/**
    \fn goToTime
    \brief called when seeking. Need to cleanup our stuff.
*/
bool         changeFps::goToTime(uint64_t usSeek)
{
    double timing=(double)usSeek;
    timing/=configuration.oldFpsNum;
    timing/=configuration.newFpsDen;
    timing*=configuration.newFpsNum;
    timing*=configuration.oldFpsDen;
    if(false==ADM_coreVideoFilter::goToTime((uint64_t)timing)) return false;
    return true;
}

/**
    \fn getCoupledConf
*/ 
bool         changeFps::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, confChangeFps_param,&configuration);
}

void changeFps::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, confChangeFps_param, &configuration);
}
/**
    \fn getNextFrame
*/
 bool         changeFps::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(false==previousFilter->getNextFrame(fn,image))
            return false;
    if(image->Pts==ADM_NO_PTS)
        return true;
    double timing=image->Pts;

    timing*=configuration.oldFpsNum;
    timing*=configuration.newFpsDen;
    timing/=configuration.newFpsNum;
    timing/=configuration.oldFpsDen;

    image->Pts=(uint64_t)timing;

    return true;
}
/**
    \fn updateTimingInfo
    \brief update the info part with new fps
*/
bool changeFps::updateTimingInfo(void)
{
    
    double fps1000=configuration.newFpsNum*1000;
    fps1000/=configuration.newFpsDen;
    // 1 update frame increment...
    info.frameIncrement=ADM_Fps1000FromUs( (uint64_t)fps1000);

    // 2 update duration
    double timing=previousFilter->getInfo()->totalDuration;
    timing*=configuration.oldFpsNum;
    timing*=configuration.newFpsDen;
    timing/=configuration.newFpsNum;
    timing/=configuration.oldFpsDen;
    info.totalDuration=(uint64_t)timing;
    return true;
}
/**
    \fn configure
*/
bool changeFps::configure(void)
{
again:
    float newFrac=configuration.newFpsNum; 
    newFrac/=configuration.newFpsDen;

    float oldFrac=configuration.oldFpsNum; 
    oldFrac/=configuration.oldFpsDen;


ADM_assert(nbPredefined == 6);
  
   diaMenuEntry tFps[]={
#define Z(x)                 {x,     predefinedFps[x].desc}
                    Z(0),Z(1),Z(2),Z(3),Z(4),Z(5)

          };
    
    

    diaElemMenu mFps(&(configuration.oldMode),   QT_TRANSLATE_NOOP("changeFps","Source Fps:"), 6,tFps);
    diaElemFloat fps(&oldFrac,QT_TRANSLATE_NOOP("changeFps","Source frame rate:"),1,200.);

    mFps.link(tFps+0,1,&fps); // only activate entry in custom mode

    diaElemMenu targetmFps(&(configuration.newMode),   QT_TRANSLATE_NOOP("changeFps","Destination Fps:"), 6,tFps);
    diaElemFloat targetfps(&newFrac,QT_TRANSLATE_NOOP("changeFps","Destination frame rate:"),1,200.);

    targetmFps.link(tFps+0,1,&targetfps); // only activate entry in custom mode



    diaElem *elems[4]={&mFps,&fps,&targetmFps,&targetfps};
  
    if( !diaFactoryRun(QT_TRANSLATE_NOOP("changeFps","Change fps"),4,elems))
        return false;
    
    if(newFrac==0 || oldFrac==0)
    {
        GUI_Error_HIG("Error","Invalid fps");
        goto again;
    }
      // 
      if(!configuration.newMode) // Custom mode
      {
          newFrac*=1000;
          configuration.newFpsNum=(uint32_t)floor(newFrac+0.4);
          configuration.newFpsDen=(uint32_t)1000;
      }else   // Preset
        {
            const PredefinedFps_t *me=&(predefinedFps[configuration.newMode]);
            configuration.newFpsNum=me->num;
            configuration.newFpsDen=me->den;
        }

    if(!configuration.oldMode) // Custom mode
      {
          oldFrac*=1000;
          configuration.oldFpsNum=(uint32_t)floor(oldFrac+0.4);
          configuration.oldFpsDen=(uint32_t)1000;
      }else   // Preset
        {
            const PredefinedFps_t *me=&(predefinedFps[configuration.oldMode]);
            configuration.oldFpsNum=me->num;
            configuration.oldFpsDen=me->den;
        }
      updateTimingInfo();
      return true;
}

//EOF
