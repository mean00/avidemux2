/** *************************************************************************
                    \fn       dummyVideoFilter.cpp  
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
#include "ADM_coreVideoFilterInternal.h"

/**
    \class dummyVideoFilter
*/
class dummyVideoFilter : public  ADM_coreVideoFilter
{
public:
                    dummyVideoFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~dummyVideoFilter();

       virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
	   virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   dummyVideoFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_MISC,            // Category
                        "dummy",            // internal name (must be uniq!)
                        "Dummy",            // Display name
                        "Null filter, it does nothing at all." // Description
                    );

// Now implements the interesting parts
/**
    \fn dummyVideoFilter
    \brief constructor
*/
dummyVideoFilter::dummyVideoFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);

    // By default the info field contains the output of previous filter
    // Tweak it here if you change fps, duration, width,...
}
/**
    \fn dummyVideoFilter
    \brief destructor
*/
dummyVideoFilter::~dummyVideoFilter()
{
		
}
/**
    \fn getFrame
    \brief Get a processed frame
*/
bool dummyVideoFilter::getNextFrame(ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    return previousFilter->getNextFrame(image);
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         dummyVideoFilter::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *dummyVideoFilter::getConfiguration(void)
{
    
    return "Dummy Filter.";
}
// Normally not needed :virtual FilterInfo  *getInfo(void)
//EOF