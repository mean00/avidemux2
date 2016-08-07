/** *************************************************************************
                    \fn       horizontalFlipFilter.cpp  
                    \brief    horizontal flip

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

/**
    \class horizontalFlipFilter
*/
class horizontalFlipFilter : public  ADM_coreVideoFilter
{
public:
                    horizontalFlipFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~horizontalFlipFilter();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   horizontalFlipFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_TRANSFORM,            // Category
                        "hflip",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("hflip","Horizontal Flip"),            // Display name
                        QT_TRANSLATE_NOOP("hflip","Horizontally flip the image.") // Description
                    );

// Now implements the interesting parts
/**
    \fn horizontalFlipFilter
    \brief constructor
*/
horizontalFlipFilter::horizontalFlipFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);

    // By default the info field contains the output of previous filter
    // Tweak it here if you change fps, duration, width,...
}
/**
    \fn horizontalFlipFilter
    \brief destructor
*/
horizontalFlipFilter::~horizontalFlipFilter()
{
		
}

static void flipLine(uint8_t *line, uint32_t w)
{
    int count=w>>1;
    uint8_t *h=line;
    uint8_t *e=line+w-1;
    while(count--)
    {
        uint8_t r=*e;
        *e=*h;
        *h=r;
        h++;e--;
    }
}

static void flipPlane(uint8_t *data, uint32_t w,uint32_t h,uint32_t stride)
{
        for( int i=0;i<h;i++)
        {
                flipLine(data,w);
                data+=stride;
        }
}
/**
    \fn getFrame
    \brief Get a processed frame
*/
bool horizontalFlipFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,image))
    {
        ADM_warning("FlipFilter : Cannot get frame\n");
        return false;
    }
    // do in place flip
    int w=info.width;
    int h=info.height;
    for(int i=0;i<3;i++)
    {
        if(i==1)
        {
            w>>=1;
            h>>=1;
        }
        flipPlane(image->GetWritePtr((ADM_PLANE)i),w,h,image->GetPitch((ADM_PLANE)i));
    }
    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         horizontalFlipFilter::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}

void horizontalFlipFilter::setCoupledConf(CONFcouple *couples)
{
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *horizontalFlipFilter::getConfiguration(void)
{
    
    return "Horizontal flip.";
}
// Normally not needed :virtual FilterInfo  *getInfo(void)
//EOF
