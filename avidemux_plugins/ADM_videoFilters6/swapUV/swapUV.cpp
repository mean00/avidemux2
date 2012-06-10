/***************************************************************************
   \file swapUV.cpp
    \author mean fixounet@free.fr (C) 2010
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
#include "DIA_coreToolkit.h"
class swapUv : public  ADM_coreVideoFilter
{
protected:
        
public:
                    swapUv(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~swapUv();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;           /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   swapUv,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_COLORS,            // Category
                        "swapUV",            // internal name (must be uniq!)
                        "Swap UV",            // Display name
                        "Swap the U and V planes." // Description
                    );

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *swapUv::getConfiguration(void)
{
    static char conf[80];
    conf[0]=0;
    snprintf(conf,80,"swap UV");
    return conf;
}
/**
    \fn ctor
*/
swapUv::swapUv( ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{	
	
	
}
/**
    \fn dtor
*/
swapUv::~swapUv()
{

}

/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         swapUv::getCoupledConf(CONFcouple **couples)
{
    *couples=NULL;
    return true;
}

void swapUv::setCoupledConf(CONFcouple *couples)
{
}

/**
    \fn getNextFrame
*/
bool swapUv::getNextFrame(uint32_t *fn,ADMImage *image)
{
    ADMImageRefWrittable ref(info.width,info.height);
 
    uint32_t strides[3];
    uint8_t  *ptr[3];
    image->GetWritePlanes(ptr);
    image->GetPitches(strides);

    ref._planes[0]=ptr[0];
    ref._planeStride[0]=strides[0];

    ref._planes[1]=ptr[2];
    ref._planeStride[1]=strides[2];

    ref._planes[2]=ptr[1];
    ref._planeStride[2]=strides[2];


    if(false==previousFilter->getNextFrame(fn,&ref))
    {
        ADM_warning("swapUV : Cannot get frame\n");
        return false;
    }
    image->Pts=ref.Pts;
    return true;
}

/**
    \fn configure
*/
bool swapUv::configure(void)
{      
          return true;     
}
//EOF
