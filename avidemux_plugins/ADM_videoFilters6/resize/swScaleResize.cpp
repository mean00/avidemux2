/** *************************************************************************
                    \fn       swScaleResizeFilter.cpp  
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
#include "DIA_factory.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
}

#include "swresize.h"
#include "swresize_desc.cpp"

typedef struct alg
{
					int in;
					char *name;
}alg;
#define DECLARE(y) {SWS_##y,(char *)#y}

/**
	Convert mplayer-resize numbering <--> avidemux one

*/
alg algs[]={
				DECLARE(BILINEAR),
				DECLARE(BICUBIC),
				DECLARE(LANCZOS),
				DECLARE(SPLINE)
		};


/**
    \class swScaleResizeFilter
*/
class swScaleResizeFilter : public  ADM_coreVideoFilter
{
protected:
            
				ADMColorScalerFull	*resizer;
				bool        reset(uint32_t nw, uint32_t old,uint32_t algo);
				bool        clean( void );
                ADMImage    *original;
                swresize    configuration;

public:
                    swScaleResizeFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~swScaleResizeFilter();

       virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
       virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
	   virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
	   virtual void setCoupledConf(CONFcouple *couples);
       virtual bool         configure(void) ;             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   swScaleResizeFilter,   // Class
                        1,0,1,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_TRANSFORM,            // Category
                        "swscale",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("resize","swsResize"),            // Display name
                        QT_TRANSLATE_NOOP("resize","swScale Resizer.") // Description
                    );

/**
    \fn swScaleResizeFilter
    \brief constructor
*/
swScaleResizeFilter::swScaleResizeFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);

    original=new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
    if(!setup || !ADM_paramLoad(setup,swresize_param,&configuration))
    {
        // Default value
        configuration.width=info.width;
        configuration.height=info.height;
        configuration.algo=1; // bicubic
        configuration.sourceAR=0;
        configuration.targetAR=0;
        configuration.lockAR=true;
        configuration.roundup=0;
    }
    resizer=NULL;
	reset(configuration.width,configuration.height,configuration.algo);
}
/**
    \fn swScaleResizeFilter
    \brief destructor
*/
swScaleResizeFilter::~swScaleResizeFilter()
{
        if(original) delete original;
        original=NULL;  
        clean();
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool swScaleResizeFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,original))
    {
        ADM_warning("swResize : Cannot get frame\n");
        return false;
    }
    uint8_t *src[3];
    uint8_t *dst[3];
    int ssrc[3];
    int ddst[3];

    original->GetReadPlanes(src);
    image->GetWritePlanes(dst);
    original->GetPitches(ssrc);
    image->GetPitches(ddst);
    resizer->convertPlanes(ssrc,ddst,src,dst);
    image->copyInfo(original);
// Fixme change A/R ?
return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         swScaleResizeFilter::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, swresize_param,&configuration);
}

void swScaleResizeFilter::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, swresize_param, &configuration);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *swScaleResizeFilter::getConfiguration(void)
{
    static char conf[80];
    conf[0]=0;
    snprintf(conf,80,"swscale Resize : %d x %d  => %d x %d, algo %d\n",
                (int)previousFilter->getInfo()->width,
                (int)previousFilter->getInfo()->height,
                (int)configuration.width, (int)configuration.height,(int)configuration.algo);
    return conf;
}
/**
    \fn getInfo
*/
FilterInfo  *swScaleResizeFilter::getInfo(void)
{
    return &info;
}
/**
    \fn clean
    \brief delete resizer
*/
bool swScaleResizeFilter::clean(void)
{
		if(resizer)
		{
			delete resizer;
		}
		resizer=NULL;
		return true;
}
/**
    \fn reset
    \brief reset resizer
*/

bool swScaleResizeFilter::reset(uint32_t nw, uint32_t nh,uint32_t algo)
{
    clean();
    ADMColorScaler_algo scalerAlgo;
    info.width=nw;
    info.height=nh;
    switch(algo)
    {
        case 0: //bilinear
                scalerAlgo=ADM_CS_BILINEAR;break;
        case 1: //bicubic
                scalerAlgo=ADM_CS_BICUBIC;break;
        case 2: //Lanczos
                scalerAlgo=ADM_CS_LANCZOS;break;
        case 3: //spline
                scalerAlgo=ADM_CS_SPLINE;break;
        default:
                ADM_error("Invalid algo: %u\n",algo);
                ADM_assert(0);
    }
    resizer=new ADMColorScalerFull(scalerAlgo, 
                        previousFilter->getInfo()->width, previousFilter->getInfo()->height, 
                        nw,nh,
                        ADM_COLOR_YV12,ADM_COLOR_YV12);
    return 1;
}

extern bool         DIA_resize(uint32_t originalWidth,uint32_t originalHeight,uint32_t fps1000,swresize *resize);
/**
    \fn configure

*/
bool         swScaleResizeFilter::configure(void) 
{
    uint32_t fps1000=ADM_Fps1000FromUs(info.frameIncrement);
    if(true==DIA_resize(previousFilter->getInfo()->width,previousFilter->getInfo()->height,
                        fps1000,&configuration))
    {
       
        reset(configuration.width,configuration.height,configuration.algo);
        return true;
    }   
    return false;
}
//EOF
