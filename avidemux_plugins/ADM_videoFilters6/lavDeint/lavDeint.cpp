/** *************************************************************************
                    \fn       lavDeint.cpp  
                    \brief simplest of all video filters, it does nothing

    copyright            : (C) 2009 by mean

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

extern "C" {
#include "libpostproc/postprocess.h"
}

#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "lav.h"
#include "lav_desc.cpp"
#include "ADM_imageFlags.h"

enum 
{
  PP_BM_NONE           =0x0000,
  PP_BM_LINEAR_BLEND   =0x0001, 
  PP_BM_LINEAR_INTER   =0x0002, 
  PP_BM_CUBIC_INTER    =0x0003, 
  PP_BM_MEDIAN_INTER   =0x0004, 
  PP_BM_FFMPEG_DEINT   =0x0005,  
};

/**
    \class lavDeint
*/
class lavDeint : public  ADM_coreVideoFilter
{
protected:
        lav                  param;
        ADMImage             *src;
        bool                 cleanup(void);
        bool                 setup(void);
        void                 *ppcontext;
        void                 *ppmode;

public:
                    lavDeint(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~lavDeint();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;     /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   lavDeint,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "lavdeint",            // internal name (must be uniq!)
                        "Libavdec Deinterlacers",            // Display name
                        "Lavcodec deinterlacer family." // Description
                    );

/**
    \fn lavDeint
    \brief constructor
*/
lavDeint::lavDeint(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
    src=NULL;
    ppcontext=NULL;
    ppmode=NULL;

    if(!setup || !ADM_paramLoad(setup,lav_param,&param))
    {
        // Default value
        param.deintType=PP_BM_FFMPEG_DEINT;
        param.autoLevel=false;
    }  	  	
    src=new ADMImageDefault(previousFilter->getInfo()->width,previousFilter->getInfo()->height);		
    this->setup();
}
/**
    \fn lavDeint
    \brief destructor
*/
lavDeint::~lavDeint()
{
      if(src) delete src;
      src=NULL;
      cleanup();
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool lavDeint::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,src))
    {
        ADM_warning("rotate : Cannot get frame\n");
        return false;
    }
 
  //
  const uint8_t *iBuff[3];
  uint8_t *oBuff[3];
  int strideIn[3],strideOut[3];
  uint32_t stride[3];

  image->GetWritePlanes(oBuff);
  src->GetReadPlanes((uint8_t **)iBuff);

  image->GetPitches(stride);
  for(int i=0;i<3;i++) strideOut[i]=stride[i];

  src->GetPitches(stride);
  for(int i=0;i<3;i++) strideIn[i]=stride[i];
        
  int type;
  if(src->flags&AVI_KEY_FRAME)
    type=1;
  else if(src->flags & AVI_B_FRAME)
    type=3;
  else
    type=2;
  pp_postprocess(
        iBuff,
        strideIn,
        oBuff,
        strideOut,
        info.width,
        info.height,
        NULL,
        0,
        ppmode,
        ppcontext,
        type); // I ?
                                
    image->copyInfo(src);
    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         lavDeint::getCoupledConf(CONFcouple **couples)
{
   return ADM_paramSave(couples, lav_param,&param);
}

void lavDeint::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, lav_param, &param);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *lavDeint::getConfiguration(void)
{
    static char buffer[80];
    snprintf(buffer,80,"lavdeint type %d, autolevel=%d.",(int)param.deintType, (int)param.autoLevel);
    return buffer;
}

/**
    \fn configure
*/
bool lavDeint::configure( void)
{

  #define PX(x) &(param.x)
   diaMenuEntry menuField[6]={{PP_BM_NONE,        QT_TR_NOOP("None"),NULL},
                             {PP_BM_LINEAR_BLEND, QT_TR_NOOP("Linear blend"),NULL},
                             {PP_BM_LINEAR_INTER, QT_TR_NOOP("Linear interpolate"),NULL},
                             {PP_BM_CUBIC_INTER,  QT_TR_NOOP("Cubic interpolate"),NULL},
                             {PP_BM_MEDIAN_INTER, QT_TR_NOOP("Median interpolate"),NULL},
                             {PP_BM_FFMPEG_DEINT, QT_TR_NOOP("FFmpeg deint"),NULL},
                          };
  
    
    diaElemMenu     menu1(PX(deintType),QT_TR_NOOP("_Deinterlacing:"), 6,menuField);
    diaElemToggle   autolevel(PX(autoLevel),QT_TR_NOOP("_Autolevel"));
    
    diaElem *elems[2]={&menu1,&autolevel};
  
   if(diaFactoryRun(QT_TR_NOOP("libavcodec deinterlacer"),2,elems))
  {
    setup();
    return true; 
  }
  return false;        
}
/**
    \fn setup
*/
bool lavDeint::setup(void)
{
  char string[1024];
  uint32_t ppCaps=0;
                
  string[0]=0;
  
        cleanup();
#ifdef ADM_CPU_X86
#define ADD(x,y) if( CpuCaps::has##x()) ppCaps|=PP_CPU_CAPS_##y;                
                ADD(MMX,MMX);           
                ADD(3DNOW,3DNOW);
                ADD(MMXEXT,MMX2);
#endif             
        cleanup();
#undef ADD       
#define ADD(z)  { if(string[0]) strcat(string,","#z); else strcpy(string,#z);}        
               
        if(param.autoLevel) ADD(al);
        switch(param.deintType)
        {
          case PP_BM_NONE:break;
          case PP_BM_LINEAR_BLEND: ADD(lb);break;
          case PP_BM_LINEAR_INTER: ADD(li);break;
          case PP_BM_CUBIC_INTER: ADD(ci);break;
          case PP_BM_MEDIAN_INTER: ADD(md);break;
          case PP_BM_FFMPEG_DEINT: ADD(fd);break;                             
        }        


        ppcontext=pp_get_context(info.width, info.height, ppCaps);           
        ppmode=pp_get_mode_by_name_and_quality(string,1);;
        
        ADM_assert(ppcontext);
        ADM_assert(ppmode);
        return true;
} 

/**
    \fn cleanup
*/
bool lavDeint::cleanup(void)
{
  if(ppcontext)
  {
    pp_free_context(ppcontext);
    ppcontext=NULL;
  }
  if(ppmode)
  {
    pp_free_mode(ppmode);
    ppmode=NULL;
  }
  return true;
} 
//EOF
