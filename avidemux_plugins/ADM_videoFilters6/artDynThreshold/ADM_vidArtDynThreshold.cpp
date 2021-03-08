/***************************************************************************
                          DynThreshold filter 
    Algorithm:
        Copyright 2021 szlldm
    Ported to Avidemux:
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
#include "DIA_factory.h"
#include "artDynThreshold.h"
#include "artDynThreshold_desc.cpp"
#include "ADM_vidArtDynThreshold.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getArtDynThreshold(artDynThreshold *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoArtDynThreshold,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoArtDynThreshold,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_ART,            // Category
                                      "artDynThreshold",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("artDynThreshold","Dynamic Threshold"),            // Display name
                                      QT_TRANSLATE_NOOP("artDynThreshold","Adaptive luma thresholding ") // Description
                                  );

/**
    \fn ArtDynThresholdProcess_C
*/
void ADMVideoArtDynThreshold::ArtDynThresholdProcess_C(ADMImage *img, uint32_t levels, float offset)
{
    if (!img) return;
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int stride;
    uint8_t * ptr;
    unsigned int hyst[256];
    unsigned int sum = width*height;
    uint8_t luma[256];

    if (levels < 2) levels = 2;
    if (levels > 16) levels = 16;
    if (offset < -1) offset = -1;
    if (offset > 1) offset = 1;

    memset(hyst, 0, 256*sizeof(unsigned int));

    if(img->_range == ADM_COL_RANGE_MPEG)
        img->expandColorRange();

    // Y plane
    // create hystogram
    stride=img->GetPitch(PLANAR_Y);
    ptr=img->GetWritePtr(PLANAR_Y);
    for(int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            hyst[ptr[x]]++;
        }
        ptr+=stride;
    }

    // create luma
    int acum = 0;
    int step = 1;
    uint8_t l = 0;
    for (int i=0; i<256; i++)
    {
        acum += hyst[i];
        if (acum > (((step+offset)*sum)/levels))
        {
            int ll = ((step*256)/(levels-1)) - 1;
            l = ((ll>255) ? 255 : ll); 
            step++;
        }
        luma[i] = l;
    }

    //apply luma
    stride=img->GetPitch(PLANAR_Y);
    ptr=img->GetWritePtr(PLANAR_Y);
    for(int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            ptr[x] = luma[ptr[x]];
        }
        ptr+=stride;
    }

    // blacken UV planes
   for (int p=1; p<3; p++)
    {
        stride=img->GetPitch((ADM_PLANE)p);
        ptr=img->GetWritePtr((ADM_PLANE)p);
        memset(ptr, 128, (height/2)*stride);
    }

}

/**
    \fn configure
*/
bool ADMVideoArtDynThreshold::configure()
{
    uint8_t r=0;

    r=  DIA_getArtDynThreshold(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoArtDynThreshold::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255,"Levels:%d, Offset:%.2f", _param.levels,_param.offset);
    return s;
}
/**
    \fn ctor
*/
ADMVideoArtDynThreshold::ADMVideoArtDynThreshold(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,artDynThreshold_param,&_param))
        reset(&_param);
    update();
}
/**
    \fn reset
*/
void ADMVideoArtDynThreshold::reset(artDynThreshold *cfg)
{
    cfg->levels = 2;
    cfg->offset = 0.0;
}
/**
    \fn valueLimit
*/
float ADMVideoArtDynThreshold::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn valueLimit
*/
int32_t ADMVideoArtDynThreshold::valueLimit(int32_t val, int32_t min, int32_t max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoArtDynThreshold::update(void)
{
    _levels=valueLimit(_param.levels,2,16);
    _offset=valueLimit(_param.offset,-1.0,1.0);
}
/**
    \fn dtor
*/
ADMVideoArtDynThreshold::~ADMVideoArtDynThreshold()
{
}
/**
    \fn getCoupledConf
*/
bool ADMVideoArtDynThreshold::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, artDynThreshold_param,&_param);
}

void ADMVideoArtDynThreshold::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, artDynThreshold_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoArtDynThreshold::getNextFrame(uint32_t *fn,ADMImage *image)
{
    /*
    ADMImage *src;
    src=vidCache->getImage(nextFrame);
    if(!src)
        return false; // EOF
    *fn=nextFrame++;
    image->copyInfo(src);
    image->copyPlane(src,image,PLANAR_Y); // Luma is untouched
    src = image;

    DoFilter(...);

    vidCache->unlockAll();
    */
    if(!previousFilter->getNextFrame(fn,image)) return false;

    ArtDynThresholdProcess_C(image, _levels, _offset);

    return 1;
}

