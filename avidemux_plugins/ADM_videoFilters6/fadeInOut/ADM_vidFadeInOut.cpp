/***************************************************************************
                          FadeInOut filter
    Copyright 2021 szlldm
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#define _USE_MATH_DEFINES // some compilers do not export M_PI etc.. if GNU_SOURCE or that is defined, let's do that
#include <cmath>
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_coreVideoFilterInternal.h"
#include "ADM_vidMisc.h"
#include "DIA_factory.h"
#include "fadeInOut.h"
#include "fadeInOut_desc.cpp"
#include "ADM_vidFadeInOut.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#if defined(FADEIN) + defined(FADEOUT) != 1
#error FADE DIRECTION NOT DEFINED!
#endif



extern uint8_t DIA_getFadeInOut(fadeInOut *param, ADM_coreVideoFilter *in);

/**
    \fn FadeDirIsIn
*/
bool ADMVideoFadeInOut::FadeDirIsIn()
{
#if defined(FADEIN)
    return true;
#else
    return false;
#endif
}
/**
    \fn FadeInOutProcess_C
*/
void ADMVideoFadeInOut::FadeInOutProcess_C(ADMImage *img, int w, int h, fadeInOut param)
{
    if (!img) return;
    uint8_t * line;

    uint32_t startMs = param.startTime;
    uint32_t endMs = param.endTime;

    if (startMs > endMs)
    {
        uint64_t tmp = endMs;
        endMs = startMs;
        startMs = tmp;
    }
    
    uint32_t imgMs = img->Pts/1000LL;
    
    if (startMs == endMs)
        return;
    if (imgMs < startMs)
        return;
    if (imgMs > endMs)
        return;

    double frac = ((double)(imgMs - startMs)) / ((double)(endMs - startMs));
#if defined(FADEIN)
    frac = 1.0 - frac;
#endif

    // rgb to yuv:
    int rgb[3], yuv[3];
    rgb[0] = (param.rgbColor>>16)&0xFF;
    rgb[1] = (param.rgbColor>>8)&0xFF;
    rgb[2] = (param.rgbColor>>0)&0xFF;
    if(img->_range == ADM_COL_RANGE_MPEG)
    {
        yuv[0] = std::round( 0.257*rgb[0] + 0.504*rgb[1] + 0.098*rgb[2]) + 16;
        yuv[1] = std::round(-0.148*rgb[0] - 0.291*rgb[1] + 0.439*rgb[2]) + 128;
        yuv[2] = std::round( 0.439*rgb[0] - 0.368*rgb[1] - 0.071*rgb[2]) + 128;
        for (int i=0; i<3; i++)
            if (yuv[i] <   16) yuv[i] = 16;
        if (yuv[0] > 235) yuv[0] = 235;
        if (yuv[1] > 240) yuv[1] = 240;
        if (yuv[2] > 240) yuv[2] = 240;
    } else {
        yuv[0] = std::round( 0.299*rgb[0] + 0.587*rgb[1] + 0.114*rgb[2]);
        yuv[1] = std::round(-0.169*rgb[0] - 0.331*rgb[1] + 0.500*rgb[2]) + 128;
        yuv[2] = std::round( 0.500*rgb[0] - 0.419*rgb[1] - 0.081*rgb[2]) + 128;
        for (int i=0; i<3; i++)
        {
            if (yuv[i] <   0) yuv[i] = 0;
            if (yuv[i] > 255) yuv[i] = 255;
        }
    }
    
    // fix swapped uv
    int uvswap = yuv[1]; yuv[1] = yuv[2]; yuv[2]=uvswap;
    
    // premultiply yuv
    for (int i=0; i<3; i++)
        yuv[i] = yuv[i]*frac*256.0 + 0.49;

    uint8_t * imgPlanes[3];
    int imgStrides[3];

    img->GetWritePlanes(imgPlanes);
    img->GetPitches(imgStrides);

    int imgblend = (1.0-frac)*256.0 + 0.49;
    for (int p=0; p<3; p++)
    {
        int width = ((p==0) ? w:(w/2));
        int height = ((p==0) ? h:(h/2));
        int px;
        uint8_t * ipl = imgPlanes[p];
        for (int y=0; y<height; y++)
        {
            for (int x=0; x<width; x++)
            {
                px = imgblend*ipl[x] + yuv[p];
                ipl[x] = px / 256;
            }
            ipl += imgStrides[p];
        }
    }
}

/**
    \fn configure
*/
bool ADMVideoFadeInOut::configure()
{
    uint8_t r=0;

    r=  DIA_getFadeInOut(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoFadeInOut::getConfiguration(void)
{
    static char s[512];
    
    char startTimeStr[128];
    char endTimeStr[128];
    snprintf(startTimeStr,127,"%s",ADM_us2plain(_param.startTime*1000LL));
    snprintf(endTimeStr,127,"%s",ADM_us2plain(_param.endTime*1000LL));

    snprintf(s,511,"%s - %s: %s #%06X",startTimeStr,endTimeStr,(FadeDirIsIn() ? "from":"to"),_param.rgbColor&0x00FFFFFF);

    return s;
}
/**
    \fn ctor
*/
ADMVideoFadeInOut::ADMVideoFadeInOut(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,fadeInOut_param,&_param))
    {
        // Default value
        _param.startTime = info.markerA / 1000LL;
        _param.endTime = info.markerB / 1000LL;
        _param.rgbColor = 0;
    }
    
    update();
}
/**
    \fn update
*/
void ADMVideoFadeInOut::update(void)
{
}
/**
    \fn dtor
*/
ADMVideoFadeInOut::~ADMVideoFadeInOut()
{
}
/**
    \fn getCoupledConf
*/
bool ADMVideoFadeInOut::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, fadeInOut_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoFadeInOut::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, fadeInOut_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoFadeInOut::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(!previousFilter->getNextFrame(fn,image)) return false;

    FadeInOutProcess_C(image,info.width,info.height,_param);
 
    return 1;
}

