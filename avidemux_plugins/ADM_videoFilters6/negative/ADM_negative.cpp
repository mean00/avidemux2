/** *************************************************************************
                    \file     negative.cpp  
                    \brief    invert Y,U or V plane
    copyright            : (C) 2011 by mean
    copyright            : (C) 2021 by szlldm

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
#include "negative.h"
#include "negative_desc.cpp"
/**
    \class negativeFilter
*/
class negativeFilter : public  ADM_coreVideoFilter
{
public:
                    negativeFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~negativeFilter();
                negative  config;
        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   negativeFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_COLORS,            // Category
                        "invplane",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("negative","Negative"),            // Display name
                        QT_TRANSLATE_NOOP("negative","Invert Y,U or V plane") // Description
                    );

// Now implements the interesting parts
/**
    \fn negativeFilter
    \brief constructor
*/
negativeFilter::negativeFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
    if(!setup || !ADM_paramLoad(setup,negative_param,&config))
    {
        // Default value
        config.invertY=true;
        config.invertU=true;
        config.invertV=true;
    }
    // By default the info field contains the output of previous filter
    // Tweak it here if you change fps, duration, width,...
}
/**
    \fn negativeFilter
    \brief destructor
*/
negativeFilter::~negativeFilter()
{
		
}
/**
    \fn blank
*/
static bool invert_plane(ADMImage *img,ADM_PLANE plane)
{
    int width=img->GetWidth(plane); 
    int height=img->GetHeight(plane);
    int stride=img->GetPitch(plane);
    uint8_t *ptr=img->GetWritePtr(plane);

    for(int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            ptr[x] ^= 255;
        }
        ptr+=stride;
    }
    return true;
}   
/**
    \fn getFrame
    \brief Get a processed frame
*/
bool negativeFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,image))
    {
        ADM_warning("Negative : Cannot get frame\n");
        return false;
    }

    if(image->_range == ADM_COL_RANGE_MPEG)
        image->expandColorRange();

    if(config.invertY)
    {
        invert_plane(image,PLANAR_Y);
    }
    if(config.invertU)
    {
        invert_plane(image,PLANAR_U);
    }
    if(config.invertV)
    {
        invert_plane(image,PLANAR_V);
    }

    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         negativeFilter::getCoupledConf(CONFcouple **couples)
{
      return ADM_paramSave(couples, negative_param,&config);
}

void negativeFilter::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, negative_param, &config);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *negativeFilter::getConfiguration(void)
{
    static char cfg[256];
    static const char *yesno[2]={"N","Y"};
    snprintf(cfg,255,"Invert Plane (Invert Y:%s Invert U:%s Invert V:%s).",
                    yesno[config.invertY],yesno[config.invertU],yesno[config.invertV]);
    return cfg;
}

/**
    \fn configure
*/
bool negativeFilter::configure(void)
{
  
  diaElemToggle planeY(&(config.invertY),QT_TRANSLATE_NOOP("negative","Invert Y Plane"),QT_TRANSLATE_NOOP("negative","Process luma plane"));
  diaElemToggle planeU(&(config.invertU),QT_TRANSLATE_NOOP("negative","Invert U Plane"),QT_TRANSLATE_NOOP("negative","Process chromaU plane"));
  diaElemToggle planeV(&(config.invertV),QT_TRANSLATE_NOOP("negative","Invert V Plane"),QT_TRANSLATE_NOOP("negative","Process chromaV plane"));
  
  
  diaElem *elems[3]={&planeY,&planeU,&planeV};
  
  return diaFactoryRun(QT_TRANSLATE_NOOP("negative","Invert plane"),3,elems);
}


//EOF
