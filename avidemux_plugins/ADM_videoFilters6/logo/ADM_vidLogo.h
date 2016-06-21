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
#pragma once
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
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
