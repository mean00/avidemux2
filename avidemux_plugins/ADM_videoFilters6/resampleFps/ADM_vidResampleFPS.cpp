/***************************************************************************
    \file ADM_vidResampleFps
    \author mean fixounet@free.fr
    \brief Simple filter that enforces output constant frame per second
    Can be used both to change fps of a movie or to enforce in case of drops
    or pulldown. In that case, the input is a mix of 24 & 30 fps, the output
    is fixed 24 fps.
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

#include "confResampleFps.h"
#include "confResampleFps_desc.cpp"

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
    \class resampleFps

*/
class  resampleFps:public ADM_coreVideoFilterCached
{
protected:
        confResampleFps     configuration;
        bool                updateIncrement(void);
        uint64_t            baseTime;
        ADMImage            *frames[2];
        bool                refill(void);   // Fetch next frame
        bool                prefillDone;        // If true we already have 2 frames fetched
public:
                            resampleFps(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~resampleFps();
        bool                goToTime(uint64_t usSeek);
        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;           /// Start graphical user interface
};
//***********************************
// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   resampleFps,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_TRANSFORM,            // Category
                        "resampleFps",            // internal name (must be uniq!)
                        "Resample FPS",            // Display name
                        "Change and enforce FPS. Keep duration and sync." // Description
                    );

/**
    \fn updateIncrement
    \brief FPS->TimeIncrement
*/
bool resampleFps::updateIncrement(void)
{
    float f=configuration.newFpsNum*1000;
    f/=configuration.newFpsDen;
    info.frameIncrement=ADM_UsecFromFps1000((uint32_t)f);
  
    return true;
}
/**
    \fn getConfiguration
*/
const char *resampleFps::getConfiguration( void )
{
static char buf[100];
 snprintf(buf,99," Resample to %2.2f fps",(double)configuration.newFpsNum/configuration.newFpsDen);
 return buf;  
}
/**
    \fn ctor
*/
resampleFps::resampleFps(  ADM_coreVideoFilter *previous,CONFcouple *setup) : 
        ADM_coreVideoFilterCached(3,previous,setup)
{
    baseTime=0;
    prefillDone=false;
    frames[0]=frames[1]=NULL;
    if(!setup || !ADM_paramLoad(setup,confResampleFps_param,&configuration))
    {
        // Default value
        configuration.mode=0;
        configuration.newFpsNum=ADM_Fps1000FromUs(previous->getInfo()->frameIncrement);
        configuration.newFpsDen=1000;
    }
    if(!frames[0]) frames[0]=new ADMImageDefault(info.width,info.height);
    if(!frames[1]) frames[1]=new ADMImageDefault(info.width,info.height);
    updateIncrement();
}
/**
    \fn dtor

*/
resampleFps::~resampleFps()
{
    if(frames[0]) delete frames[0];
    if(frames[1]) delete frames[1];
    frames[0]=frames[1]=NULL;
}
/**
     \fn refill
     \brief fetch a new frame from source, shift the old one as "old"
    frames[0]=old
    frames[1]=new
*/
bool resampleFps::refill(void)
{
    ADMImage *nw=frames[0];
    uint32_t img=0;
    frames[0]=frames[1];
    frames[1]=nw;
    return previousFilter->getNextFrame(&img,nw);
}
/**
    \fn goToTime
    \brief called when seeking. Need to cleanup our stuff.
*/
bool         resampleFps::goToTime(uint64_t usSeek)
{
    if(false==ADM_coreVideoFilterCached::goToTime(usSeek)) return false;
    prefillDone=false;
    return true;
}

/**
    \fn getCoupledConf
*/ 
bool         resampleFps::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, confResampleFps_param,&configuration);
}

void resampleFps::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, confResampleFps_param, &configuration);
}

/**
    \fn getNextFrame
*/
 bool         resampleFps::getNextFrame(uint32_t *fn,ADMImage *image)
{

    if(!prefillDone) // Empty, need 1/ to refill, 2/ to rebase
    {
          if(false==refill()) return false;
          baseTime=frames[1]->Pts;  // We start at the first frame
          if(false==refill()) return false;
          prefillDone=true;
    }
    float offset=configuration.newFpsDen;
    offset*=1000000LL;
    offset*=nextFrame;
    offset/=configuration.newFpsNum;
    uint64_t thisTime=baseTime+(uint64_t)offset;

again:
    
    uint64_t frame1Dts=frames[0]->Pts;
    uint64_t frame2Dts=frames[1]->Pts;
    aprintf("Frame : %d, timeIncrement %d ms, Wanted : %"LLU", available %"LLU" and %"LLU"\n",
                    nextFrame,info.frameIncrement/1000,thisTime,frame1Dts,frame2Dts);
    if(thisTime>frame1Dts && thisTime>frame2Dts)
    {
        if(false==refill()) return false;
        goto again;
    }
    if(thisTime<frame1Dts && thisTime<frame2Dts)
    {
        image->duplicate(frames[0]);
        image->Pts=thisTime;
        *fn=nextFrame++;
        return true;
    }
    // In between, take closer
    double diff1=(double)thisTime-double(frame1Dts);
    double diff2=(double)thisTime-double(frame2Dts);
    if(diff1<0) diff1=-diff1;
    if(diff2<0) diff2=-diff2;
    int index=1;
    if(diff1<diff2) index=0;

    image->duplicate(frames[index]);
    image->Pts=thisTime;
    *fn=nextFrame++;
    return true;
}
#if 0
  ADMImage *mysrc1=NULL;
  ADMImage *mysrc2=NULL;

  if(frame>=_info.nb_frames) return 0;
  // read uncompressed frame
  
  // What frame are we seeking ?
  double f;
  uint32_t page=_info.width*_info.height;
  
  f=frame;
  f*=_in->getInfo()->fps1000;
  f/=_param->newfps;
  
  if(!_param->use_linear)
  {
      uint32_t nw;
      
      nw=(uint32_t)floor(f+0.4);
      if(nw>_in->getInfo()->nb_frames-1)
        nw=_in->getInfo()->nb_frames-1;
    
      mysrc1=vidCache->getImage(nw);
      if(!mysrc1) return 0;
      
      memcpy(YPLANE(data),YPLANE(mysrc1),page);
      memcpy(UPLANE(data),UPLANE(mysrc1),page>>2);
      memcpy(VPLANE(data),VPLANE(mysrc1),page>>2);
    
      vidCache->unlockAll();
      
      return 1;
  }
  /* With linear blending */
  uint32_t nw;
  uint8_t lowweight;
  uint8_t highweight;
  
  double diff;
  
  nw=(uint32_t)floor(f);
  diff=f-floor(f);
  highweight = (uint8_t)floor(diff*256);
  lowweight = 256 - highweight;

  if(nw>=_in->getInfo()->nb_frames-1)
    {
      printf("[ResampleFps] In %u Out %u\n",frame,nw);
      nw=_in->getInfo()->nb_frames-1;
      highweight=0;
    }
  //printf("New:%lu old:%lu\n",frame,nw);

  if(highweight == 0)
    {
      mysrc1=vidCache->getImage(nw);  
      if(!mysrc1) return 0;
      
      memcpy(YPLANE(data),YPLANE(mysrc1),page);
      memcpy(UPLANE(data),UPLANE(mysrc1),page>>2);
      memcpy(VPLANE(data),VPLANE(mysrc1),page>>2);
      
      vidCache->unlockAll();
    }
  else
    {
      mysrc1=vidCache->getImage(nw);
      mysrc2=vidCache->getImage(nw+1);
      if(!mysrc1 || !mysrc2) return 0;
      
      uint8_t *out, *in1, *in2;
      uint32_t count;
      uint32_t idx;
      
      out = YPLANE(data);
      in1 = YPLANE(mysrc1);
      in2 = YPLANE(mysrc2);
        
      count = page;

#ifdef ADM_CPU_X86
        if(CpuCaps::hasMMX())
                blendMMX(in1,in2,out,lowweight,highweight,(count*3)>>1);
        else
#endif
      {
      for(idx = 0; idx < count; ++idx)
	out[idx] = ((in1[idx]*lowweight) + (in2[idx]*highweight))>>8;

      out = UPLANE(data);
      in1 = UPLANE(mysrc1);
      in2 = UPLANE(mysrc2);
      count = page>>2;

      for(idx = 0; idx < count; ++idx)
        out[idx] = ((in1[idx]*lowweight) + (in2[idx]*highweight))>>8;      


      out = VPLANE(data);
      in1 = VPLANE(mysrc1);
      in2 = VPLANE(mysrc2);
      count = page>>2;

      for(idx = 0; idx < count; ++idx)
	out[idx] = ((in1[idx]*lowweight) + (in2[idx]*highweight))>>8;
      }

      vidCache->unlockAll();
    }
  return 1;
}
#endif 
/**
    \fn configure
*/
bool resampleFps::configure(void)
{

    float f=configuration.newFpsNum; 
    f/=configuration.newFpsDen;

ADM_assert(nbPredefined == 6);
  
   diaMenuEntry tFps[]={
#define Z(x)                 {x,     predefinedFps[x].desc}
                    Z(0),Z(1),Z(2),Z(3),Z(4),Z(5)

          };
    uint32_t sel=configuration.mode;
    

    diaElemMenu mFps(&(configuration.mode),   QT_TR_NOOP("_Mode:"), 6,tFps);
    diaElemFloat fps(&f,QT_TR_NOOP("_New frame rate:"),1,200.);

    mFps.link(tFps+0,1,&fps); // only activate entry in custom mode

    diaElem *elems[2]={&mFps,&fps};
  
    if( diaFactoryRun(QT_TR_NOOP("Resample fps"),2,elems))
    {
      if(!configuration.mode) // Custom mode
      {
          f*=1000;
          configuration.newFpsNum=(uint32_t)floor(f+0.4);
          configuration.newFpsDen=(uint32_t)1000;
      }else   // Preset
        {
            const PredefinedFps_t *me=&(predefinedFps[configuration.mode]);
            configuration.newFpsNum=me->num;
            configuration.newFpsDen=me->den;
        }
      prefillDone=false;
      updateIncrement();
      return 1;
    }
    return 0;
}

//EOF
