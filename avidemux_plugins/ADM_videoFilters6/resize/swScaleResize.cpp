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
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
extern "C" {
#include "ADM_ffmpeg/libavcodec/avcodec.h"
#include "ADM_ffmpeg/libavutil/avutil.h"
#include "ADM_ffmpeg/libswscale/swscale.h"
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
				DECLARE(LANCZOS)
		};


/**
    \class swScaleResizeFilter
*/
class swScaleResizeFilter : public  ADM_coreVideoFilter
{
protected:
            
				SwsContext	*_context;
				bool        reset(uint32_t nw, uint32_t old,uint32_t algo);
				bool        clean( void );
                ADMImage    *original;
                swresize    configuration;

public:
                    swScaleResizeFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~swScaleResizeFilter();

       virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
       virtual bool         getFrame(uint32_t frame,ADMImage *image);    /// Return the next image
       virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
	   virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual bool         configure(void) ;             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   swScaleResizeFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_TRANSFORM,            // Category
                        "swscale",            // internal name (must be uniq!)
                        "swsResize",            // Display name
                        "swScale Resizer." // Description
                    );

// Now implements the interesting parts
/**
    \fn swScaleResizeFilter
    \brief constructor
*/
swScaleResizeFilter::swScaleResizeFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);

    original=new ADMImage(in->getInfo()->width,in->getInfo()->height);
    if(!setup || !ADM_paramLoad(setup,swresize_param,&configuration))
    {
        // Default value
        configuration.width=info.width;
        configuration.height=info.height;
        configuration.algo=SWS_BILINEAR;
        configuration.sourceAR=1;
        configuration.targetAR=1;
    }
    info.width=configuration.width;
    info.height=configuration.height;
    _context=NULL;
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
bool swScaleResizeFilter::getFrame(uint32_t frame,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getFrame(frame,original))
    {
        ADM_warning("Vertical flip : Cannot get frame\n");
        return false;
    }
    uint8_t *src[3];
    uint8_t *dst[3];
    int ssrc[3];
    int ddst[3];

    uint32_t page;

    page=original->_width*original->_height;
    src[0]=YPLANE(original);
    src[1]=UPLANE(original);
    src[2]=VPLANE(original);

    ssrc[0]=original->_width;
    ssrc[1]=ssrc[2]=original->_width>>1;

    page=info.width*info.height;
    dst[0]=YPLANE(image);
    dst[1]=UPLANE(image);
    dst[2]=VPLANE(image);
    ddst[0]=info.width;
    ddst[1]=ddst[2]=info.width>>1;

    sws_scale(_context,src,ssrc,0,original->_height,dst,ddst);
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
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *swScaleResizeFilter::getConfiguration(void)
{
    static char conf[80];
    conf[0]=0;
    snprintf(conf,80,"swscale Resize : %"LU"x%"LU" => %"LU"x%"LU", algo %"LU"\n",
                previousFilter->getInfo()->width,
                previousFilter->getInfo()->height,
                configuration.width, configuration.height,configuration.algo);
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
		if(_context)
		{
			sws_freeContext(_context);
		}
		_context=NULL;
		return true;
}
/**
    \fn reset
    \brief reset resizer
*/

bool swScaleResizeFilter::reset(uint32_t nw, uint32_t old,uint32_t algo)
{
 				 SwsFilter 		    *srcFilter=NULL;
				 SwsFilter		    *dstFilter=NULL;
				 int 			   flags=0;


				clean();
                flags=algo;
                switch(flags)
                {
                    case 0: //bilinear
                            flags=SWS_BILINEAR;break;
                    case 1: //bicubic
                            flags=SWS_BICUBIC;break;
                    case 2: //Lanczos
                            flags=SWS_LANCZOS;break;
                    default:ADM_assert(0);

                }
#ifdef ADM_CPU_X86		
		#define ADD(x,y) if( CpuCaps::has##x()) flags|=SWS_CPU_CAPS_##y;
		ADD(MMX,MMX);		
		ADD(3DNOW,3DNOW);
		ADD(MMXEXT,MMX2);
#endif	
#ifdef ADM_CPU_ALTIVEC
		flags|=SWS_CPU_CAPS_ALTIVEC;
#endif
                _context=sws_getContext(
                        previousFilter->getInfo()->width,previousFilter->getInfo()->height,
                    PIX_FMT_YUV420P,
                    nw,old,
                    PIX_FMT_YUV420P,
                        flags, srcFilter, dstFilter,NULL);

				if(!_context) return 0;
				return 1;


}

extern bool         DIA_resize(uint32_t originalWidth,uint32_t originalHeight,uint32_t fps1000,swresize *resize);
/**
    \fn configure

*/
bool         swScaleResizeFilter::configure(void) 
{
    uint32_t fps1000=ADM_Fps1000FromUs(info.frameIncrement);
    return DIA_resize(previousFilter->getInfo()->width,previousFilter->getInfo()->height,
                        fps1000,&configuration);

}
//EOF
