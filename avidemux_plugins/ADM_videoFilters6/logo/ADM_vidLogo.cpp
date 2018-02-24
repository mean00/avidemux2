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
#include "ADM_vidLogo.h"
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   addLogopFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_TRANSFORM,            // Category
                        "addLogo",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("logo","Add logo"),            // Display name
                        QT_TRANSLATE_NOOP("logo","Put a logo on top of video, with alpha blending.") // Description
                    );

extern bool DIA_getLogo(logo *param, ADM_coreVideoFilter *in);




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
        configuration.logoImageFile=std::string("");
        configuration.fade=0;
    }
    in->getTimeRange(&startOffset,&endOffset);
    from=in->getAbsoluteStartTime();
    myName="logo";
    reloadImage();
}
/**

*/
bool addLogopFilter::reloadImage(void)
{
        if(myImage) delete myImage;
        myImage=NULL;

        if(!configuration.logoImageFile.size())
        {
            return false;
        }
        myImage=createImageFromFile(configuration.logoImageFile.c_str());
        if(!myImage) return false;
        return true;
}
/**
    \fn addLogopFilter
    \brief destructor
*/
addLogopFilter::~addLogopFilter()
{
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
    if(myImage)
    {
        double a=(double)configuration.alpha;
        uint64_t pts=image->Pts;
        pts+=from;
        uint64_t transition=configuration.fade*1000LL;
        uint64_t duration=endOffset-startOffset;
        if(transition && duration)
        {
            if(transition*2 > duration)
                transition=duration/2;
            if(pts < startOffset || pts >= endOffset)
            {
                a = 0.;
            }else
            {
                pts -= startOffset;
                if(pts < transition)
                {
                    a /= (double)transition;
                    a *= pts;
                }
                if(pts > duration-transition)
                {
                    a /= (double)transition;
                    a *= duration-pts;
                }
            }
            if(a > 255.)
                a = 255.;
        }
        if(myImage->GetReadPtr(PLANAR_ALPHA))
            myImage->copyWithAlphaChannel(image,configuration.x,configuration.y,(uint32_t)a);
        else
            myImage->copyToAlpha(image,configuration.x,configuration.y,(uint32_t)a);
    }
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
    static char c[2560];
    snprintf(c,2559,"X: %d; Y: %d; Alpha: %d; Fade-in/out: %d ms;\nimage: %s",
            configuration.x,configuration.y,configuration.alpha,configuration.fade,
            configuration.logoImageFile.c_str());
    return c;

}

/**
    \fn configure
*/
bool addLogopFilter::configure( void)
{
    bool r=DIA_getLogo(&configuration, this->previousFilter);
    reloadImage();
    return r;
}


/************************************************/
//EOF
