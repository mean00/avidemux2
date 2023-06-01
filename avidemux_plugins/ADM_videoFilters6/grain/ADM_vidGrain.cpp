/***************************************************************************
                          Grain filter 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "grain.h"
#include "grain_desc.cpp"
#include "ADM_vidGrain.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getGrain(grain *param, ADM_coreVideoFilter *in);

// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoGrain,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoGrain,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_ART,            // Category
                                      "grain",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("grain","Grain"),            // Display name
                                      QT_TRANSLATE_NOOP("grain","Film grain effect.") // Description
                                  );
/**
    \fn GrainProcess_C
*/
void ADMVideoGrain::GrainProcess_C(ADMImage *img, float noise)
{
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int stride;
    uint8_t * ptr;

    uint32_t rng_state;
    uint32_t rn;
    uint64_t rng_product;
    
    rng_state = img->Pts;
    if (rng_state == 0)
        rng_state = 123456789;
    
    int32_t nm = 1045*noise;
    // Y plane
    stride=img->GetPitch(PLANAR_Y);
    ptr=img->GetWritePtr(PLANAR_Y);
    for(int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            int pixel = ptr[x];
            
            rng_product = (uint64_t)rng_state * 48271;
            rng_state = (rng_product & 0x7fffffff) + (rng_product >> 31);
            rng_state = (rng_state & 0x7fffffff) + (rng_state >> 31);
            rn = rng_state & 0x0FFF;
            
            int32_t r = rn;
            r = (r*62259 + 1638)/65536;
            int32_t q = r - 2048;
            r = (q*q)/4096;
            int32_t m = (-34757*r + 9897)/16;
            int32_t n = (r*r*16 - 49855*r + 8657)/65536;
            if (n==0) n=1;
            int32_t o = m/n;
            n = o + 5591;
            n *= q;
            n /= 16384;
            n *= nm;
            n /= 16384;

            pixel += n;
            
            pixel = ((pixel < 0) ? 0 : ((pixel > 255) ? 255: pixel));
            ptr[x] = pixel;
        }
        ptr+=stride;
    }
}

/**
    \fn configure
*/
bool ADMVideoGrain::configure()
{
    uint8_t r=0;

    r=  DIA_getGrain(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoGrain::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Noise: %.2f",_param.noise);
    return s;
}
/**
    \fn ctor
*/
ADMVideoGrain::ADMVideoGrain(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,grain_param,&_param))
        reset(&_param);
    update();
}
/**
    \fn reset
*/
void ADMVideoGrain::reset(grain *cfg)
{
    cfg->noise = 0.05;
}
/**
    \fn valueLimit
*/
float ADMVideoGrain::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoGrain::update(void)
{
    _noise=valueLimit(_param.noise, 0.0, 1.0);
}
/**
    \fn dtor
*/
ADMVideoGrain::~ADMVideoGrain()
{
    
}
/**
    \fn getCoupledConf
*/
bool ADMVideoGrain::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, grain_param,&_param);
}

void ADMVideoGrain::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, grain_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoGrain::getNextFrame(uint32_t *fn,ADMImage *image)
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

    GrainProcess_C(image, _noise);

    return 1;
}

