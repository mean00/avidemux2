/** *************************************************************************
                    \file     removePlaneFilter.cpp  
                    \brief    remove Y,U or V plane
    copyright            : (C) 2011 by mean

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
#include "DIA_factory.h"
#include "rplane.h"
#include "rplane_desc.cpp"
/**
    \class removePlaneFilter
*/
class removePlaneFilter : public  ADM_coreVideoFilter
{
public:
                    removePlaneFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~removePlaneFilter();
                removePlane  config;
        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   removePlaneFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_COLORS,            // Category
                        "rplane",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("removeplane","Remove  Plane"),            // Display name
                        QT_TRANSLATE_NOOP("removeplane","Remove Y,U or V plane (used mainly to debug other filters).") // Description
                    );

// Now implements the interesting parts
/**
    \fn removePlaneFilter
    \brief constructor
*/
removePlaneFilter::removePlaneFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
    if(!setup || !ADM_paramLoad(setup,removePlane_param,&config))
    {
        // Default value
        config.keepY=true;
        config.keepU=true;
        config.keepV=true;
    }
    // By default the info field contains the output of previous filter
    // Tweak it here if you change fps, duration, width,...
}
/**
    \fn removePlaneFilter
    \brief destructor
*/
removePlaneFilter::~removePlaneFilter()
{
		
}
/**
    \fn blank
*/
static bool blank(ADMImage *img,ADM_PLANE plane, int filler)
{
    int width=img->GetWidth(plane); 
    int height=img->GetHeight(plane);
    int stride=img->GetPitch(plane);
    uint8_t *ptr=img->GetWritePtr(plane);

    for(int y=0;y<height;y++)
    {
        memset(ptr,filler,width);
        ptr+=stride;
    }
    return true;
}   
/**
    \fn getFrame
    \brief Get a processed frame
*/
bool removePlaneFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,image))
    {
        ADM_warning("FlipFilter : Cannot get frame\n");
        return false;
    }
    if(!config.keepY)
    {   // blank Y
        blank(image,PLANAR_Y,128);
    }
    if(!config.keepU)
    {
        blank(image,PLANAR_U,128);
    }
    if(!config.keepV)
    {
        blank(image,PLANAR_V,128);
    }

    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         removePlaneFilter::getCoupledConf(CONFcouple **couples)
{
      return ADM_paramSave(couples, removePlane_param,&config);
}

void removePlaneFilter::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, removePlane_param, &config);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *removePlaneFilter::getConfiguration(void)
{
    static char cfg[256];
    static const char *yesno[2]={"N","Y"};
    snprintf(cfg,255,"Remove Plane (Keep Y:%s Keep U:%s Keep V:%s).",
                    yesno[config.keepY],yesno[config.keepU],yesno[config.keepV]);
    return cfg;
}

/**
    \fn configure
*/
bool removePlaneFilter::configure(void)
{
  
  diaElemToggle planeY(&(config.keepY),QT_TRANSLATE_NOOP("removeplane","Keep Y Plane"),QT_TRANSLATE_NOOP("removeplane","Process luma plane"));
  diaElemToggle planeU(&(config.keepU),QT_TRANSLATE_NOOP("removeplane","Keep U Plane"),QT_TRANSLATE_NOOP("removeplane","Process chromaU plane"));
  diaElemToggle planeV(&(config.keepV),QT_TRANSLATE_NOOP("removeplane","Keep V Plane"),QT_TRANSLATE_NOOP("removeplane","Process chromaV plane"));
  
  
  diaElem *elems[3]={&planeY,&planeU,&planeV};
  
  return diaFactoryRun(QT_TRANSLATE_NOOP("removeplane","Remove plane"),3,elems);
}


//EOF
