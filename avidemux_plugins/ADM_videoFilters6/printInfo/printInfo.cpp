/** *************************************************************************
                    \fn       printInfoFilter.cpp  
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
#include "ADM_vidMisc.h"
/**
    \class printInfoFilter
*/
class printInfoFilter : public  ADM_coreVideoFilter
{
public:
                    printInfoFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~printInfoFilter();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *frameNumner,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   printInfoFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_MISC,            // Category
                        "printInfo",            // internal name (must be uniq!)
                        "PrintInfo",            // Display name
                        "Display some informations on Screen." // Description
                    );

// Now implements the interesting parts
/**
    \fn printInfoFilter
    \brief constructor
*/
printInfoFilter::printInfoFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);

    // By default the info field contains the output of previous filter
    // Tweak it here if you change fps, duration, width,...
}
/**
    \fn printInfoFilter
    \brief destructor
*/
printInfoFilter::~printInfoFilter()
{
		
}
/**
    \fn getFrame
    \brief Get a processed frame
*/
bool printInfoFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,image))
    {
        return false;
    }
#define INFO_MAX 1024
    char info[INFO_MAX+1];
    snprintf(info,INFO_MAX,"Frame number : %06d",(int)*fn);
    image->printString(0,1,info);

    if(image->Pts==ADM_NO_PTS)
        sprintf(info,"Pts : No info");
    else
    snprintf(info,INFO_MAX,"Pts : %s",ADM_us2plain(image->Pts));
    image->printString(0,3,info);

    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         printInfoFilter::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}

void printInfoFilter::setCoupledConf(CONFcouple *couples)
{
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *printInfoFilter::getConfiguration(void)
{
    
    return "Dummy Filter.";
}
// Normally not needed :virtual FilterInfo  *getInfo(void)
//EOF
