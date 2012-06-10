/***************************************************************************
    \file ADM_vidCrop
    \brief Crop Filter
    \author Mean 2002, fixounet@free.Fr

          Crop top/left/right/bottom
          Each one ,must be even

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
#include "DIA_factory.h"
#include "crop.h"
#include "crop_desc.cpp"

/**
    \class CropFilter
*/
class  CropFilter:public ADM_coreVideoFilter
 {

 protected:
                crop           configuration;
                ADMImage       *original;

 public:

                                CropFilter(  ADM_coreVideoFilter *in,CONFcouple *couples);
        virtual                 ~CropFilter();

       virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
       //virtual FilterInfo  *getInfo(void);                    /// Return picture parameters after this filter
	   virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
	   virtual void setCoupledConf(CONFcouple *couples);
       virtual bool         configure(void) ;                 /// Start graphical user interface
    
 };
 

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   CropFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_TRANSFORM,            // Category
                        "crop",            // internal name (must be uniq!)
                        "crop",            // Display name
                        "crop filter" // Description
                    );


//_______________________________________________________________
/**
    \fn CropFilter
*/
CropFilter::CropFilter(ADM_coreVideoFilter *in,CONFcouple *couples) :ADM_coreVideoFilter(in,couples)
{

        original=new ADMImageDefault(info.width,info.height);
        if(!couples || !ADM_paramLoad(couples,crop_param,&configuration))
		{
            // Default value
            configuration.top=0;
            configuration.bottom=0;
            configuration.left=0;
            configuration.right=0;
        }
        if(  in->getInfo()->width<(configuration.right+configuration.left))
                {
                    ADM_warning("Warning Cropping too much width ! Width reseted !\n");
                        configuration.right=configuration.left=0;
                }
        if(  in->getInfo()->height<(configuration.bottom+configuration.top))
                {
                    ADM_warning("Warning Cropping too much height ! Height reseted !\n");
                    configuration.bottom=configuration.top=0;
                }

        info.width= in->getInfo()->width- configuration.right- configuration.left;		
        info.height=in->getInfo()->height-configuration.bottom-configuration.top;	
}
/**
    \fn ~CropFilter
*/
CropFilter::~CropFilter()
{
    if(original) delete original;
    original=NULL;
}

/**
    \fn getNextFrame

*/
bool         CropFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
FilterInfo  *prevInfo=previousFilter->getInfo();
			// read uncompressed frame
       		if(!previousFilter->getNextFrame(fn,original)) return false;
       		
       		// Crop Y luma
       		uint32_t y,x,line;
       		uint8_t *src,*src2,*dest;
       		
       		y=prevInfo->height;
       		x=prevInfo->width;
       		line=info.width;
       		src=YPLANE(original)+configuration.top*x+configuration.left;
       		dest=YPLANE(image);
       		
            

            for(int i=0;i<3;i++)
            {
                    uint32_t srcPitch=original->GetPitch((ADM_PLANE )i);
                    uint32_t dstPitch=image->GetPitch((ADM_PLANE )i);
                    uint8_t  *src=original->GetReadPtr((ADM_PLANE)i);
                    uint8_t  *dst=image->GetWritePtr((ADM_PLANE)i);
                    uint32_t w=image->_width;
                    uint32_t h=image->_height;

                    uint32_t wOffset=configuration.left;
                    uint32_t hOffset=configuration.top;

                    if(i)
                    {
                        w>>=1;
                        h>>=1;
                        wOffset>>=1;
                        hOffset>>=1;
                    }
                    src+=wOffset+hOffset*srcPitch;
                    BitBlit(dst, dstPitch,
                        src,srcPitch,
                        w,h);
            }
            //printf("Cropt:Dts = %"LLU"\n",image->Pts);
            image->copyInfo(original);     
            return 1;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         CropFilter::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, crop_param,&configuration);
}

void CropFilter::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, crop_param, &configuration);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *CropFilter::getConfiguration(void)
{
    static char conf[80];
    conf[0]=0;
    snprintf(conf,80,"Crop : %"LU"x%"LU" => %"LU"x%"LU,
                previousFilter->getInfo()->width,
                previousFilter->getInfo()->height,
                info.width,info.height
                );
    return conf;
}
/**
    \fn Configure
*/
extern int DIA_getCropParams(	const char *name,crop *param,ADM_coreVideoFilter *in);

bool CropFilter::configure(void)

{
		uint8_t r;
		uint32_t w,h;
    	if(r = (DIA_getCropParams("Crop Settings",&configuration,previousFilter )))
    	{
			w=configuration.left+configuration.right;
			h=configuration.top+configuration.bottom;
			ADM_assert(w<previousFilter->getInfo()->width);
			ADM_assert(h<previousFilter->getInfo()->height);
			info.width=previousFilter->getInfo()->width-w;
			info.height=previousFilter->getInfo()->height-h;
		}
		return r;
}
// EOF
