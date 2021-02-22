/** *************************************************************************
                    \fn       ADM_vidFitToSize.cpp  

    copyright            : (C) 2009 by mean
                               2021 szlldm

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

#include "fitToSize.h"
#include "fitToSize_desc.cpp"
#include "ADM_vidFitToSize.h"

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



// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ADMVideoFitToSize,   // Class
                        1,0,1,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_TRANSFORM,            // Category
                        "fitToSize",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("fitToSize","Fit to size"),            // Display name
                        QT_TRANSLATE_NOOP("fitToSize","Resize and pad to the specified size.") // Description
                    );

/**
    \fn getFitParameters
*/
void ADMVideoFitToSize::getFitParameters(int inW, int inH, int outW, int outH, float tolerance, int * strW, int * strH, int * padLeft, int * padRight, int * padTop, int * padBottom)
{
    float inAR,outAR;
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
    \fn ADMVideoFitToSize
    \brief constructor
*/
ADMVideoFitToSize::ADMVideoFitToSize(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);

    original=new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
    echo=new ADMImageDefault(16,16);
    if(!setup || !ADM_paramLoad(setup,fitToSize_param,&configuration))
    {
        // Default value
        configuration.width=info.width;
        configuration.height=info.height;
        configuration.algo=1; // bicubic
        configuration.roundup=0;
        configuration.pad=0;
        configuration.tolerance=0.0;
    }
    resizer=NULL;
    resizerOrigToEcho=NULL;
    resizerEchoToImage=NULL;
    stretch=NULL;
    reset(configuration.width,configuration.height,configuration.algo,configuration.tolerance);
}
/**
    \fn ADMVideoFitToSize
    \brief destructor
*/
ADMVideoFitToSize::~ADMVideoFitToSize()
{
    if(original) delete original;
    original=NULL;
    if (echo) delete echo;
    echo=NULL;
    clean();
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool ADMVideoFitToSize::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,original))
    {
        ADM_warning("fitToSize : Cannot get frame\n");
        return false;
    }
    int padding = configuration.pad;
    uint8_t *src[3];
    uint8_t *dst[3];
    int stridesrc[3];
    int stridedst[3];

    original->GetReadPlanes(src);
    stretch->GetWritePlanes(dst);
    original->GetPitches(stridesrc);
    stretch->GetPitches(stridedst);
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


// Fixme change A/R ?
return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         ADMVideoFitToSize::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, fitToSize_param,&configuration);
}

void ADMVideoFitToSize::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, fitToSize_param, &configuration);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *ADMVideoFitToSize::getConfiguration(void)
{
    static char conf[256];
    conf[0]=0;

    snprintf(conf,255,"Fit %d x %d to %d x %d, algo %d, pad method %d\nResize input to: %d x %d, Padding: [%d,..,%d] x [%d,..,%d]",
                (int)previousFilter->getInfo()->width,
                (int)previousFilter->getInfo()->height,
                (int)configuration.width, (int)configuration.height,(int)configuration.algo,(int)configuration.pad,
                stretchW,stretchH, pads[0], pads[1], pads[2], pads[3] );
    return conf;
}
/**
    \fn getInfo
*/
FilterInfo  *ADMVideoFitToSize::getInfo(void)
{
    return &info;
}
/**
    \fn clean
    \brief delete resizer
*/
bool ADMVideoFitToSize::clean(void)
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

bool ADMVideoFitToSize::reset(uint32_t nw, uint32_t nh,uint32_t algo, float tolerance)
{
    clean();
    ADMColorScaler_algo scalerAlgo;
    info.width=nw;
    info.height=nh;

    getFitParameters(previousFilter->getInfo()->width, previousFilter->getInfo()->height, nw, nh, tolerance, &stretchW, &stretchH, pads+0, pads+1, pads+2, pads+3);

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
                        stretchW,stretchH,
                        ADM_COLOR_YV12,ADM_COLOR_YV12);
    resizerOrigToEcho=new ADMColorScalerFull(ADM_CS_BICUBIC, 
                        previousFilter->getInfo()->width, previousFilter->getInfo()->height, 
                        16,16,
                        ADM_COLOR_YV12,ADM_COLOR_YV12);
    resizerEchoToImage=new ADMColorScalerFull(ADM_CS_LANCZOS, 
                        16,16, 
                        nw,nh,
                        ADM_COLOR_YV12,ADM_COLOR_YV12);
    stretch=new ADMImageDefault(stretchW,stretchH);
    return 1;
}

extern bool         DIA_fitToSize(uint32_t originalWidth,uint32_t originalHeight,fitToSize *param);
/**
    \fn configure

*/
bool ADMVideoFitToSize::configure(void) 
{
    if(true==DIA_fitToSize(previousFilter->getInfo()->width,previousFilter->getInfo()->height,&configuration))
    {
       
        reset(configuration.width,configuration.height,configuration.algo,configuration.tolerance);
        return true;
    }   
    return false;
}
//EOF
