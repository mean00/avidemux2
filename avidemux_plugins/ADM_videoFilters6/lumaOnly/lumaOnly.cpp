/** *************************************************************************
                    \fn       lumaOnlyFilter.cpp  
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
#include "ADM_coreVideoFilter.h"

/**
    \class lumaOnlyFilter
*/
class lumaOnlyFilter : public  ADM_coreVideoFilter
{
public:
                    lumaOnlyFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~lumaOnlyFilter();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   lumaOnlyFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_TRANSFORM,            // Category
                        "lumaonly",            // internal name (must be uniq!)
                        "GreyScale",            // Display name
                        "Remove color, only key grey image." // Description
                    );

// Now implements the interesting parts
/**
    \fn lumaOnlyFilter
    \brief constructor
*/
lumaOnlyFilter::lumaOnlyFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);

    // By default the info field contains the output of previous filter
    // Tweak it here if you change fps, duration, width,...
}
/**
    \fn lumaOnlyFilter
    \brief destructor
*/
lumaOnlyFilter::~lumaOnlyFilter()
{
		
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool lumaOnlyFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,image))
    {
        ADM_warning("lumaOnlyFilter : Cannot get frame\n");
        return false;
    }
    // do in place flip
    int w=info.width;
    int h=info.height;
    uint32_t pitches[3];
    uint8_t *ptr[3];
    image->GetPitches(pitches);
    image->GetWritePlanes((uint8_t **)ptr);
    w>>=1,
    h>>=1;
    for(int i=1;i<3;i++)
    {
        uint8_t *p=ptr[i];
        uint32_t d=pitches[i];
        for(int y=0;y<h;y++)
        {
                memset(p,128,w);
                p+=d;
        }
    }
    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         lumaOnlyFilter::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}

void lumaOnlyFilter::setCoupledConf(CONFcouple *couples)
{
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *lumaOnlyFilter::getConfiguration(void)
{
    
    return "Greyscale.";
}
// Normally not needed :virtual FilterInfo  *getInfo(void)
//EOF
