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
#include "ADM_coreVideoFilterInternal.h"
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
 {QT_TR_NOOP("Custom"),         10000,1000},
 {QT_TR_NOOP("25  (PAL)"),      25000,1000},
 {QT_TR_NOOP("23.976 (Film)"),  24000,1001},
 {QT_TR_NOOP("29.97 (NTSC)"),   30000,1001},
 {QT_TR_NOOP("50 (Pal)"),       50000,1000},
 {QT_TR_NOOP("59.93  (NTSC)"),  60000,1001}
};

#define nbPredefined (sizeof(predefinedFps)/sizeof(PredefinedFps_t))

/**
    \class changeFps

*/
class  changeFps:public ADM_coreVideoFilter
{
protected:
        confChangeFps       configuration;
public:
                            changeFps(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~changeFps();
        bool                goToTime(uint64_t usSeek);
        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
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
#warning TODO rescale time
    if(false==ADM_coreVideoFilter::goToTime(usSeek)) return false;
    return true;
}

/**
    \fn getCoupledConf
*/ 
bool         changeFps::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, confChangeFps_param,&configuration);
}
/**
    \fn getNextFrame
*/
 bool         changeFps::getNextFrame(uint32_t *fn,ADMImage *image)
{


    return true;
}

/**
    \fn configure
*/
bool changeFps::configure(void)
{

    float f=configuration.newFpsNum; 
    f/=configuration.newFpsDen;

    float f2=configuration.oldFpsNum; 
    f2/=configuration.oldFpsDen;


ADM_assert(nbPredefined == 6);
  
   diaMenuEntry tFps[]={
#define Z(x)                 {x,     predefinedFps[x].desc}
                    Z(0),Z(1),Z(2),Z(3),Z(4),Z(5)

          };
    
    

    diaElemMenu mFps(&(configuration.oldMode),   QT_TR_NOOP("Source Fps:"), 6,tFps);
    diaElemFloat fps(&f2,QT_TR_NOOP("Source frame rate:"),1,200.);

    mFps.link(tFps+0,1,&fps); // only activate entry in custom mode

    diaElemMenu targetmFps(&(configuration.newMode),   QT_TR_NOOP("Source Fps:"), 6,tFps);
    diaElemFloat targetfps(&f,QT_TR_NOOP("Source frame rate:"),1,200.);

    targetmFps.link(tFps+0,1,&targetfps); // only activate entry in custom mode



    diaElem *elems[4]={&mFps,&fps,&targetmFps,&targetfps};
  
    if( !diaFactoryRun(QT_TR_NOOP("Change fps"),4,elems))
        return false;
    
      // 
      if(!configuration.newMode) // Custom mode
      {
          f*=1000;
          configuration.newFpsNum=(uint32_t)floor(f+0.4);
          configuration.newFpsDen=(uint32_t)1000;
      }else   // Preset
        {
            const PredefinedFps_t *me=&(predefinedFps[configuration.newMode]);
            configuration.newFpsNum=me->num;
            configuration.newFpsDen=me->den;
        }

    if(!configuration.oldMode) // Custom mode
      {
          f2*=1000;
          configuration.oldFpsNum=(uint32_t)floor(f2+0.4);
          configuration.oldFpsDen=(uint32_t)1000;
      }else   // Preset
        {
            const PredefinedFps_t *me=&(predefinedFps[configuration.oldMode]);
            configuration.oldFpsNum=me->num;
            configuration.oldFpsDen=me->den;
        }

      return true;
}

//EOF
