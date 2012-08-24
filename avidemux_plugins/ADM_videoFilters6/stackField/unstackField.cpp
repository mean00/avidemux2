/** *************************************************************************
                    \fn       unstackFieldFilter.cpp  
                    \brief 

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
    \class unstackFieldFilter
*/
class unstackFieldFilter : public  ADM_coreVideoFilter
{
protected:
        ADMImage            *current;
public:
                
                    unstackFieldFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~unstackFieldFilter();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   unstackFieldFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "unstackField",            // internal name (must be uniq!)
                        "Unstack Fields",            // Display name
                        "Weave top and bottom halves." // Description
                    );

// Now implements the interesting parts
/**
    \fn unstackFieldFilter
    \brief constructor
*/
unstackFieldFilter::unstackFieldFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);

    // By default the info field contains the output of previous filter
    // Tweak it here if you change fps, duration, width,...
    current=new ADMImageDefault(info.width,info.height);
}
/**
    \fn unstackFieldFilter
    \brief destructor
*/
unstackFieldFilter::~unstackFieldFilter()
{
   delete current;
   current=NULL;
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool unstackFieldFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,current))
    {
        ADM_warning("unstackField : Cannot get frame\n");
        return false;
    }
    // do in place flip
    image->copyInfo(current);
    for(int i=PLANAR_Y;i<PLANAR_LAST;i++)
    {
        ADM_PLANE plane=(ADM_PLANE)i;
        uint32_t srcPitch=current->GetPitch(plane);
        uint32_t dstPitch=image->GetPitch(plane);
        uint8_t  *src=current->GetReadPtr(plane);
        uint8_t  *dst=image->GetWritePtr(plane);
        uint32_t w=info.width;
        uint32_t h=info.height;
        if(plane)
                {
                        w>>=1;
                        h>>=1;
                }
        // Even
        BitBlit(dst, dstPitch*2,src,srcPitch,w,h/2);
        BitBlit(dst+dstPitch, dstPitch*2,src+(srcPitch*h)/2,srcPitch,w,h/2);
        
    }
    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         unstackFieldFilter::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}

void unstackFieldFilter::setCoupledConf(CONFcouple *couples)
{
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *unstackFieldFilter::getConfiguration(void)
{
    
    return "Unstack Field.";
}
// Normally not needed :virtual FilterInfo  *getInfo(void)
//EOF
