/***************************************************************************

		Put a logon on video

    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr
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
#include "logo.h"
#include "logo_desc.cpp"
#include "ADM_imageLoader.h"
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
/**
    \class addLogopFilter
*/
class addLogopFilter : public  ADM_coreVideoFilter
{
protected:
                ADMImage    *myImage;
                logo        configuration;
                bool        reloadImage(void);
public:
                    addLogopFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~addLogopFilter();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;     /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void);                           /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   addLogopFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_TRANSFORM,            // Category
                        "addLogo",            // internal name (must be uniq!)
                        "Add logo.",            // Display name
                        "Put a logo on top of video, with alpha blending." // Description
                    );

// Now implements the interesting parts
/**
    \fn addLogopFilter
    \brief constructor
*/
addLogopFilter::addLogopFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
    myImage=NULL;
    if(!setup || !ADM_paramLoad(setup,logo_param,&configuration))
    {
        // Default value
        configuration.x=0;
        configuration.y=0;
        configuration.alpha=255;
        configuration.logo=NULL;
    }
    myName="logo";
    reloadImage();
}
/**

*/
bool addLogopFilter::reloadImage(void)
{
        if(myImage) delete myImage;
        myImage=NULL;

        if(!configuration.logo)
        {
            return false;
        }
        myImage=createImageFromFile(configuration.logo);
        if(!myImage) return false;
        return true;
}
/**
    \fn addLogopFilter
    \brief destructor
*/
addLogopFilter::~addLogopFilter()
{
    if(configuration.logo) ADM_dealloc(configuration.logo);
    configuration.logo=NULL;
    if(myImage) delete myImage;
    myImage=NULL;
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool addLogopFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,image))
    {
        ADM_warning("logoFilter : Cannot get frame\n");
        return false;
    }
    // do in place flip
#if 1
    if(myImage)
        myImage->copyToAlpha(image,configuration.x,configuration.y,configuration.alpha);
#endif
    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         addLogopFilter::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, logo_param,&configuration);
}

void addLogopFilter::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, logo_param, &configuration);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *addLogopFilter::getConfiguration(void)
{
    return "Add logo.";
}

/**
    \fn configure
*/
bool addLogopFilter::configure( void)
{
#define PX(x) &(configuration.x)
	   diaElemFile       file(0,(char **)PX(logo),QT_TR_NOOP("_Logo (jpg file):"), NULL, QT_TR_NOOP("Select JPEG file"));
	   diaElemUInteger   positionX(PX(x),QT_TR_NOOP("_X Position:"),0,info.width);
	   diaElemUInteger   positionY(PX(y),QT_TR_NOOP("_Y Position:"),0,info.height);
	   diaElemUInteger   alpha(PX(alpha),QT_TR_NOOP("_Alpha:"),0,255);

	   diaElem *elems[4]={&file,&positionX,&positionY,&alpha};

	   if( diaFactoryRun(QT_TR_NOOP("Logo"),4,elems))
	   {
		   if(false==reloadImage())
            GUI_Error_HIG("Oops","Cannot load the logo");
		   return true;
	   }
	   return false;
}


/************************************************/
//EOF
