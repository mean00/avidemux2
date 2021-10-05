/***************************************************************************
    \file ADM_vidZoom
    \brief Zoom Filter
    \author Mean 2002, fixounet@free.Fr
            szlldm 2021

          Zoom top/left/right/bottom
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
#include "ADM_image.h"
#include "ADM_coreVideoFilter.h"
#include "DIA_factory.h"
#include "zoom.h"
#include "zoom_desc.cpp"
#include "math.h"

/**
    \class ZoomFilter
*/
class  ZoomFilter:public ADM_coreVideoFilter
 {
  protected:
    zoom                 configuration;
    ADMImage *           original;
    void                 resetConfig(void);
    void                 getFitParameters(int inW, int inH, int outW, int outH, float tolerance, int * strW, int * strH, int * padLeft, int * padRight, int * padTop, int * padBottom);
    ADMColorScalerFull * resizer;
    bool                 reset(int left, int right, int top, int bottom, uint32_t algo, float tolerance);
    bool                 clean( void );
    ADMImage *           stretch;
    ADMImage *           echo;
    ADMColorScalerFull * resizerOrigToEcho;
    ADMColorScalerFull * resizerEchoToImage;
    int                  stretchW;
    int                  stretchH;
    int                  pads[4];

  public:
                         ZoomFilter(  ADM_coreVideoFilter *in,CONFcouple *couples);
    virtual             ~ZoomFilter();

    virtual const char * getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    //virtual FilterInfo  *getInfo(void);                    /// Return picture parameters after this filter
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface
 };
 

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ZoomFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_TRANSFORM,            // Category
                        "zoom",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("zoom","Zoom"),            // Display name
                        QT_TRANSLATE_NOOP("zoom","Partializable crop filter.") // Description
                    );


//_______________________________________________________________
/**
    \fn ZoomFilter
*/
ZoomFilter::ZoomFilter(ADM_coreVideoFilter *in,CONFcouple *couples) :ADM_coreVideoFilter(in,couples)
{
    original=new ADMImageDefault(info.width,info.height);
    echo=new ADMImageDefault(16,16);
    resetConfig();
    if(couples && !ADM_paramLoadPartial(couples,zoom_param,&configuration))
    {
        resetConfig();
    }
    if(info.width < (configuration.right+configuration.left))
    {
        ADM_warning("Cropped width for zoom exceeds image width. Resetting left and right crop values.\n");
        configuration.right=configuration.left=0;
    }
    if(info.height < (configuration.bottom+configuration.top))
    {
        ADM_warning("Cropped height for zoom exceeds image height. Resetting top and bottom crop values.\n");
        configuration.bottom=configuration.top=0;
    }

    resizer=NULL;
    resizerOrigToEcho=NULL;
    resizerEchoToImage=NULL;
    stretch=NULL;
    reset(configuration.left,configuration.right,configuration.top,configuration.bottom,configuration.algo,configuration.tolerance);

    ADM_info("%s\n",getConfiguration());
}
/**
    \fn ~ZoomFilter
*/
ZoomFilter::~ZoomFilter()
{
    if(original) delete original;
    original=NULL;
    if (echo) delete echo;
    echo=NULL;
    clean();
}
/**
    \fn getFitParameters
*/
void ZoomFilter::getFitParameters(int inW, int inH, int outW, int outH, float tolerance, int * strW, int * strH, int * padLeft, int * padRight, int * padTop, int * padBottom)
{
    float inAR,outAR;
    if (inW < 0)
        inW = 0;
    if (inH < 0)
        inH = 0;
    inAR  = (float)inW/(float)inH;
    outAR = (float)outW/(float)outH;

    //stretching
    if (inAR > outAR)    // vertical stretching/padding required
    {
        if (inAR <= outAR*(1.0+tolerance)) {    // no padding required
            *strW = outW;
            *strH = outH;
        } else {
            *strW = outW;
            *strH = round(((float)outW/inAR)/2.0)*2;
        }
    } else {             // horizontal stretching/padding required
        if (inAR*(1.0+tolerance) >= outAR) {    // no padding required
            *strW = outW;
            *strH = outH;
        } else {
            *strH = outH;
            *strW = round(((float)outH*inAR)/2.0)*2;
        }
    }

    //safety checks
    if (*strW > outW) *strW = outW;
    if (*strH > outH) *strH = outH;
    if (*strW < 16) *strW = 16;
    if (*strH < 16) *strH = 16;

    // padding
    *padLeft = 0;
    *padRight = 0;
    *padTop = 0;
    *padBottom = 0;
    if (*strW < outW)
    {
        if ((outW-*strW) < 4)    // no padding
        {
            *strW = outW;
        } else {
            int diff = outW-*strW;
            *padLeft = (diff/4)*2;
            *padRight = diff-*padLeft;
        }
    }

    if (*strH < outH)
    {
        if ((outH-*strH) < 4)    // no padding
        {
            *strH = outH;
        } else {
            int diff = outH-*strH;
            *padTop = (diff/4)*2;
            *padBottom = diff-*padTop;
        }
    }
}
/**
    \fn clean
    \brief delete resizer
*/
bool ZoomFilter::clean(void)
{
    if (resizer) delete resizer;
    resizer=NULL;
    if (stretch) delete stretch;
    stretch=NULL;
    if (resizerOrigToEcho) delete resizerOrigToEcho;
    resizerOrigToEcho=NULL;
    if (resizerEchoToImage) delete resizerEchoToImage;
    resizerEchoToImage=NULL;
    return true;
}
/**
    \fn reset
    \brief reset resizer
*/

bool ZoomFilter::reset(int left, int right, int top, int bottom, uint32_t algo, float tolerance)
{
    clean();
    ADMColorScaler_algo scalerAlgo;

    getFitParameters(info.width - (left+right), info.height - (top+bottom), info.width, info.height, tolerance, &stretchW, &stretchH, pads+0, pads+1, pads+2, pads+3);

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
                        info.width - (left+right), info.height - (top+bottom),
                        stretchW,stretchH,
                        ADM_PIXFRMT_YV12,ADM_PIXFRMT_YV12);
    resizerOrigToEcho=new ADMColorScalerFull(ADM_CS_BICUBIC, 
                        info.width - (left+right), info.height - (top+bottom),
                        16,16,
                        ADM_PIXFRMT_YV12,ADM_PIXFRMT_YV12);
    resizerEchoToImage=new ADMColorScalerFull(ADM_CS_LANCZOS, 
                        16,16, 
                        info.width, info.height,
                        ADM_PIXFRMT_YV12,ADM_PIXFRMT_YV12);
    stretch=new ADMImageDefault(stretchW,stretchH);
    return 1;
}
/**
   \fn resetConfig
*/
void ZoomFilter::resetConfig(void)
{
    configuration.top=0;
    configuration.bottom=0;
    configuration.left=0;
    configuration.right=0;
    configuration.ar_select=0;
    configuration.algo=1;
    configuration.tolerance=0.01;
    configuration.pad=0;
}
/**
    \fn getNextFrame

*/
bool ZoomFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    FilterInfo  *prevInfo=previousFilter->getInfo();
    // read uncompressed frame
    if(!previousFilter->getNextFrame(fn,original)) return false;

    int padding = configuration.pad;
    uint8_t *src[3];
    uint8_t *dst[3];
    int stridesrc[3];
    int stridedst[3];

    original->GetReadPlanes(src);
    stretch->GetWritePlanes(dst);
    original->GetPitches(stridesrc);
    stretch->GetPitches(stridedst);
    src[0] += (configuration.left + stridesrc[0]*configuration.top);
    src[1] += (configuration.left/2 + stridesrc[1]*(configuration.top/2));
    src[2] += (configuration.left/2 + stridesrc[2]*(configuration.top/2));
    resizer->convertPlanes(stridesrc,stridedst,src,dst);

    if (padding == 1)    // echo mode
    {
        echo->GetWritePlanes(dst);
        echo->GetPitches(stridedst);
        resizerOrigToEcho->convertPlanes(stridesrc,stridedst,src,dst);
    }

    image->GetWritePlanes(dst);
    image->GetPitches(stridedst);

    if (padding == 1)    // echo mode
    {
        echo->GetReadPlanes(src);
        echo->GetPitches(stridesrc);
        resizerEchoToImage->convertPlanes(stridesrc,stridedst,src,dst);
    }

    stretch->GetReadPlanes(src);
    stretch->GetPitches(stridesrc);

    int srcwidth=stretchW;
    int srcheight=stretchH;
    int width=image->GetWidth(PLANAR_Y); 
    int height=image->GetHeight(PLANAR_Y);
    int pleft = pads[0];
    int pright = pads[1];
    int ptop = pads[2];
    int pbot = pads[3];
    int blacklevel = ((original->_range == ADM_COL_RANGE_MPEG) ? 16:0);
    int p,y;


    for (p=0; p<3;p++)
    {
        for (y=0; y<ptop; y++)
        {
            if (padding == 0)
                memset(dst[p], blacklevel, width);

            dst[p] += stridedst[p];
        }

        for (y=0; y<srcheight; y++)
        {
            if (padding == 0)
                memset(dst[p], blacklevel, pleft);

            memcpy(dst[p]+pleft, src[p], srcwidth);

            if (padding == 0)
                memset(dst[p]+pleft+srcwidth, blacklevel, pright);

            src[p] += stridesrc[p];
            dst[p] += stridedst[p];
        }

        for (y=0; y<pbot; y++)
        {
            if (padding == 0)
                memset(dst[p], blacklevel, width);

            dst[p] += stridedst[p];
        }

        if (p == 0)
        {
            srcwidth /= 2;
            srcheight /= 2;
            width /= 2;
            height /= 2;
            pleft /= 2;
            pright /= 2;
            ptop /= 2;
            pbot /= 2;
            blacklevel = 128;
        }
    }
    image->copyInfo(original);     
    return 1;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool ZoomFilter::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, zoom_param,&configuration);
}

void ZoomFilter::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, zoom_param, &configuration);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *ZoomFilter::getConfiguration(void)
{
    static char conf[128];
    conf[0]=0;
    snprintf(conf,127,"Selection %dx%d => %dx%d",
                (int)info.width - (configuration.right+configuration.left),
                (int)info.height - (configuration.bottom+configuration.top),
                (int)info.width,(int)info.height
                );

    return conf;
}
/**
    \fn Configure
*/
extern int DIA_getZoomParams(	const char *name,zoom *param,ADM_coreVideoFilter *in);

bool ZoomFilter::configure(void)

{
    uint8_t r;
    uint32_t w,h;
    r = DIA_getZoomParams("Zoom Settings",&configuration,previousFilter );
    if(r)
    {
        w=configuration.left+configuration.right;
        h=configuration.top+configuration.bottom;
        ADM_assert(w<previousFilter->getInfo()->width);
        ADM_assert(h<previousFilter->getInfo()->height);
        info.width=previousFilter->getInfo()->width;
        info.height=previousFilter->getInfo()->height;
        ADM_info("%s\n",getConfiguration());
        reset(configuration.left,configuration.right,configuration.top,configuration.bottom,configuration.algo,configuration.tolerance);
    }
    return r;
}
// EOF
