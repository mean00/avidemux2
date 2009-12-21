/** *************************************************************************
                    \fn       verticalFlipFilter.cpp  
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
    \class verticalFlipFilter
*/
class verticalFlipFilter : public  ADM_coreVideoFilter
{
public:
                    verticalFlipFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~verticalFlipFilter();

       virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
	   virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   verticalFlipFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_TRANSFORM,            // Category
                        "vflip",            // internal name (must be uniq!)
                        "Vertical Flip",            // Display name
                        "Vertically flip the image." // Description
                    );

// Now implements the interesting parts
/**
    \fn verticalFlipFilter
    \brief constructor
*/
verticalFlipFilter::verticalFlipFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);

    // By default the info field contains the output of previous filter
    // Tweak it here if you change fps, duration, width,...
}
/**
    \fn verticalFlipFilter
    \brief destructor
*/
verticalFlipFilter::~verticalFlipFilter()
{
		
}

static void flipMe(uint8_t *data, uint32_t w,uint32_t h)
{
    uint8_t scratch[w];
    int count=h>>1;
    for(int i=0;i<count;i++)
    {
        uint8_t *top=data+w*i;
        uint8_t *bottom=data+(h-i-1)*w;
        memcpy(scratch, top,w);
        memcpy(top, bottom,w);
        memcpy(bottom,scratch,w);
    }
}
/**
    \fn getFrame
    \brief Get a processed frame
*/
bool verticalFlipFilter::getNextFrame(ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(image))
    {
        ADM_warning("FlipFilter : Cannot get frame\n");
        return false;
    }
    // do in place flip
    int w=info.width;
    int h=info.height;
    flipMe(YPLANE(image),w,h);
    flipMe(UPLANE(image),w>>1,h>>1);
    flipMe(VPLANE(image),w>>1,h>>1);
    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         verticalFlipFilter::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *verticalFlipFilter::getConfiguration(void)
{
    
    return "Vertical flip.";
}
// Normally not needed :virtual FilterInfo  *getInfo(void)
//EOF
