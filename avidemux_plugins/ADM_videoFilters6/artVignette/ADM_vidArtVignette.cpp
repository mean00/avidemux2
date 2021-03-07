/***************************************************************************
                          Vignette filter 
    Algorithm:
        Copyright (C) 2011 Simon Andreas Eugster (simon.eu@gmail.com)
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
#include "artVignette.h"
#include "artVignette_desc.cpp"
#include "ADM_vidArtVignette.h"

#ifndef M_PI
#define M_PI   3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

extern uint8_t DIA_getArtVignette(artVignette *param, ADM_coreVideoFilter *in);

// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoArtVignette,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoArtVignette,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_ART,            // Category
                                      "artVignette",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("artVignette","Vignette"),            // Display name
                                      QT_TRANSLATE_NOOP("artVignette","Lens vignetting effect, ported from frei0r.") // Description
                                  );
/**
    \fn ArtVignetteCreateMask
*/
void ADMVideoArtVignette::ArtVignetteCreateMask(float * mask, int w, int h, float aspect, float center, float soft)
{
    if (!mask) return;
    float scaleX = 1;
    float scaleY = 1;
    float scale = std::fabs(aspect-.5)*2;
    scale = 1 + 4*std::pow(scale, 3);
    soft = 5*std::pow(float(1)-soft,2)+.01;
    if (aspect > 0.5) {
        scaleX = scale;
    } else {
        scaleY = scale;
    }
    int cx = w/2;
    int cy = h/2;
    float rmax = std::sqrt(std::pow(float(cx), 2) + std::pow(float(cy), 2));
    float r,xsq,ysq,cos4;
    //for (int y = 0; y < h; y++) {
    //    for (int x = 0; x < w; x++) {
    // speed up 4x by exploiting H-W symmetries (by szlldm)
    for (int y = 0; y < cy; y++) {
        for (int x = 0; x < cx; x++) {

            // Euclidian distance to the center, normalized to [0,1]
            xsq = scaleX*(x-cx);
            xsq = xsq * xsq;
            ysq = scaleY*(y-cy);
            ysq = ysq * ysq;
            r = std::sqrt(xsq + ysq)/rmax;

            // Subtract the clear center
            r -= center;

            if (r <= 0) {
                // Clear center: Do not modify the brightness here
                mask[y*w+x] = 1;
            } else {
                r *= soft;
                if (r > M_PI_2) {
                    mask[y*w+x] = 0;
                } else {
                    cos4 = std::cos(r);
                    cos4 = cos4 * cos4;
                    cos4 = cos4 * cos4;	// power of 4
                    mask[y*w+x] = cos4; //std::pow(std::cos(r), 4);
                }
            }

        }
    }

    // make top half
    for (int y = 0; y < cy; y++) {
        for (int x = 0; x < cx; x++) {
             mask[y*w+((w-1) - x)] = mask[y*w+x];
        }
    }

    // mirror down to bottom
    for (int y = 0; y < cy; y++) {
        memcpy(&mask[((h-1) - y)*w],&mask[y*w],sizeof(float)*w);
    }
}
/**
    \fn ArtVignetteProcess_C
*/
void ADMVideoArtVignette::ArtVignetteProcess_C(ADMImage *img, float * mask)
{
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int stride;
    uint8_t * ptr;
    float pixel;

    if(img->_range == ADM_COL_RANGE_MPEG)
        img->expandColorRange();

    // Y plane
    stride=img->GetPitch(PLANAR_Y);
    ptr=img->GetWritePtr(PLANAR_Y);
    for(int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            pixel = ptr[x];
            ptr[x] = (uint8_t)std::round(pixel * mask[x+y*width]);
        }
        ptr+=stride;
    }

    // UV planes
    for (int p=1; p<3; p++)
    {
        stride=img->GetPitch((ADM_PLANE)p);
        ptr=img->GetWritePtr((ADM_PLANE)p);
        for(int y=0;y<height/2;y++)	// 4:2:0
        {
            for (int x=0;x<width/2;x++)
            {
                pixel = ptr[x];
                pixel -= 128;
                ptr[x] = (uint8_t)std::round( (pixel * mask[x*2+y*2*width]) + 128);
            }
            ptr+=stride;
        }
    }
}

/**
    \fn configure
*/
bool ADMVideoArtVignette::configure()
{
    uint8_t r=0;

    r=  DIA_getArtVignette(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoArtVignette::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Aspect ratio: %.2f, Size of the unaffected center: %.2f, Softness: %.2f",_param.aspect,_param.center,_param.soft);
    return s;
}
/**
    \fn ctor
*/
ADMVideoArtVignette::ADMVideoArtVignette(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,artVignette_param,&_param))
        reset(&_param);
    _filterMask = new float[info.width * info.height];
    update();
}
/**
    \fn reset
*/
void ADMVideoArtVignette::reset(artVignette *cfg)
{
    cfg->aspect = .5;
    cfg->center = .0;
    cfg->soft = .6;
}
/**
    \fn update
*/
void ADMVideoArtVignette::update(void)
{
    _aspect=_param.aspect;
    _center=_param.center;
    _soft=_param.soft;
    ArtVignetteCreateMask(_filterMask, info.width, info.height, _aspect, _center, _soft);
}
/**
    \fn dtor
*/
ADMVideoArtVignette::~ADMVideoArtVignette()
{
    if (_filterMask) delete _filterMask;
}
/**
    \fn getCoupledConf
*/
bool ADMVideoArtVignette::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, artVignette_param,&_param);
}

void ADMVideoArtVignette::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, artVignette_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoArtVignette::getNextFrame(uint32_t *fn,ADMImage *image)
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

    ArtVignetteProcess_C(image, _filterMask);

    return 1;
}

