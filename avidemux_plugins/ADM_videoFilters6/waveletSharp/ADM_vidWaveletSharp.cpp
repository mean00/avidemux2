/***************************************************************************
                          waveletSharp filter 
    Algorithm:
        Copyright 2008 Marco Rossini
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
#include "waveletSharp.h"
#include "waveletSharp_desc.cpp"
#include "ADM_vidWaveletSharp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getWaveletSharp(waveletSharp *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoWaveletSharp,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoWaveletSharp,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_SHARPNESS,            // Category
                                      "waveletSharp",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("waveletSharp","Wavelet sharpener"),            // Display name
                                      QT_TRANSLATE_NOOP("waveletSharp","Based on Marco Rossini's sharpener.") // Description
                                  );

/**
    \fn WaveletSharpProcess_HatTransformHorizontal
*/
inline void ADMVideoWaveletSharp::WaveletSharpProcess_HatTransformHorizontal(int32_t *temp, int32_t *base, int size, int sc)
{
    int i;
    for (i = 0; i < sc; i++)
        temp[i] = 2 * base[i] + base[sc - i] + base[i + sc];
    for (; i + sc < size; i++)
        temp[i] = 2 * base[i] + base[i - sc] + base[i + sc];
    for (; i < size; i++)
        temp[i] = 2 * base[i] + base[i - sc] + base[2 * size - 2 - (i + sc)];
}
/**
    \fn WaveletSharpProcess_HatTransformVertical
*/
inline void ADMVideoWaveletSharp::WaveletSharpProcess_HatTransformVertical(int i, int32_t *temp, int32_t *base, int st, int size, int sc)
{
    if (i < sc)
        temp[0] = (2 * base[st * i] + base[st * (sc - i)] + base[st * (i + sc)])>>4;
    else if (i + sc < size)
        temp[0] = (2 * base[st * i] + base[st * (i - sc)] + base[st * (i + sc)])>>4;
    else if (i < size)
        temp[0] = (2 * base[st * i] + base[st * (i - sc)] + base[st * (2 * size - 2 - (i + sc))])>>4;
}
/**
    \fn WaveletSharpProcess_Core
*/
void ADMVideoWaveletSharp::WaveletSharpProcess_Core(int32_t *buf[4], int levels, unsigned int width, unsigned int height, double amount, double radius, double cutoff)
{
    int32_t amt;
    unsigned int i, lev, minDim, sc, lpass, hpass, size, col, row;
    minDim = ((width < height) ? width : height);
    size = width * height;
    hpass = 0;
    int cutoffint = cutoff * 100.0;

    for (lev = 0; lev < levels; lev++)
    {
        sc = (1 << lev);
        if (sc*2 >= minDim) break;
        lpass = ((lev & 1) + 1);
        for (row = 0; row < height; row++)
        {
            WaveletSharpProcess_HatTransformHorizontal(buf[lpass] + row * width, buf[hpass] + row * width, width, sc);
        }
        for (row = 0; row < height; row++)
        {
            for (col = 0; col < width; col++)
            {
                WaveletSharpProcess_HatTransformVertical(row, buf[3]+ row * width + col, buf[lpass] + col, width, height, sc);
            }
        }
        memcpy(buf[lpass],buf[3],size*sizeof(int32_t));

        amt = std::round((amount * exp (-(lev - radius) * (lev - radius) / 1.5) + 1) * (1 << 8));

        for (i = 0; i < size; i++)
        {
            buf[hpass][i] -= buf[lpass][i];
            if (abs(buf[hpass][i]) > cutoffint)
            {
                buf[hpass][i] *= amt;
                buf[hpass][i] >>= 8;
            }

            if (hpass)
                buf[0][i] += buf[hpass][i];
        }
        hpass = lpass;
    }

    for (i = 0; i < size; i++)
    {
        buf[0][i] = buf[0][i] + buf[lpass][i];
    }
}
/**
    \fn WaveletSharpProcess_C
*/
void ADMVideoWaveletSharp::WaveletSharpProcess_C(ADMImage *img, float strength, float radius, float cutoff, bool highq)
{
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int stride;
    uint8_t * ptr;
    int pixel;
    int limitL = 0;
    int limitH = 255;

    int32_t * buf = (int32_t *)malloc(sizeof(int32_t) * width * height * 4);
    if (!buf) return;

    if (strength < 0.0) strength = 0.0;
    if (strength > 1.0) strength = 1.0;
    if (radius < 0.0) radius = 0.0;
    if (radius > 2.0) radius = 2.0;
    if (cutoff < 0.0) cutoff = 0.0;
    if (cutoff > 10.0) cutoff = 10.0;

    if(img->_range == ADM_COL_RANGE_MPEG)
    {
        limitL = 16;
        limitH = 235;
    }

    strength = (strength*strength)*16.0;

    int32_t *buffer[4];
    buffer[0] = &buf[width * height * 0];
    buffer[1] = &buf[width * height * 1];
    buffer[2] = &buf[width * height * 2];
    buffer[3] = &buf[width * height * 3];
    int32_t * bufptr;

    // Y plane
    stride=img->GetPitch(PLANAR_Y);
    ptr=img->GetWritePtr(PLANAR_Y);
    bufptr = buffer[0];
    for(int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            pixel = ptr[x];
            *bufptr = pixel << 8;
            bufptr++;
        }
        ptr+=stride;
    }

    // Algo
    WaveletSharpProcess_Core(buffer, (highq ? 5:3), width, height, strength, radius, cutoff);

    // Y plane
    stride=img->GetPitch(PLANAR_Y);
    ptr=img->GetWritePtr(PLANAR_Y);
    bufptr = buffer[0];
    for(int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            pixel = *bufptr >> 8;
            bufptr++;
            if (pixel < limitL) pixel = limitL;
            if (pixel > limitH) pixel = limitH;
            ptr[x] = pixel;
        }
        ptr+=stride;
    }

    free(buf);
}

/**
    \fn configure
*/
bool ADMVideoWaveletSharp::configure()
{
    uint8_t r=0;

    r=  DIA_getWaveletSharp(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoWaveletSharp::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Strength: %.2f, Radius: %.2f, Cutoff: %.2f%s",_param.strength, _param.radius, _param.cutoff,(_param.highq ? ", High quality":""));
    return s;
}
/**
    \fn ctor
*/
ADMVideoWaveletSharp::ADMVideoWaveletSharp(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,waveletSharp_param,&_param))
        reset(&_param);
    update();
}
/**
    \fn reset
*/
void ADMVideoWaveletSharp::reset(waveletSharp *cfg)
{
    cfg->strength = 0.0;
    cfg->radius = 0.5;
    cfg->cutoff = 0.0;
    cfg->highq = false;
}
/**
    \fn valueLimit
*/
float ADMVideoWaveletSharp::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn valueLimit
*/
int32_t ADMVideoWaveletSharp::valueLimit(int32_t val, int32_t min, int32_t max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoWaveletSharp::update(void)
{
    _strength=valueLimit(_param.strength,0.0,1.0);
    _radius=valueLimit(_param.radius,0.0,2.0);
    _cutoff=valueLimit(_param.cutoff,0.0,10.0);
    _highq=_param.highq;
}
/**
    \fn dtor
*/
ADMVideoWaveletSharp::~ADMVideoWaveletSharp()
{
    
}
/**
    \fn getCoupledConf
*/
bool ADMVideoWaveletSharp::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, waveletSharp_param,&_param);
}

void ADMVideoWaveletSharp::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, waveletSharp_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoWaveletSharp::getNextFrame(uint32_t *fn,ADMImage *image)
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

    WaveletSharpProcess_C(image, _strength, _radius, _cutoff, _highq);

    return 1;
}

