/***************************************************************************
                          waveletDenoise filter 
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
#include "waveletDenoise.h"
#include "waveletDenoise_desc.cpp"
#include "ADM_vidWaveletDenoise.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getWaveletDenoise(waveletDenoise *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoWaveletDenoise,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoWaveletDenoise,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_NOISE,            // Category
                                      "waveletDenoise",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("waveletDenoise","Wavelet denoiser"),            // Display name
                                      QT_TRANSLATE_NOOP("waveletDenoise","Based on Marco Rossini's denoiser.") // Description
                                  );

/**
    \fn WaveletDenoiseProcess_HatTransformHorizontal
*/
inline void ADMVideoWaveletDenoise::WaveletDenoiseProcess_HatTransformHorizontal(int32_t *temp, int32_t *base, int size, int sc)
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
    \fn WaveletDenoiseProcess_HatTransformVertical
*/
inline void ADMVideoWaveletDenoise::WaveletDenoiseProcess_HatTransformVertical(int i, int32_t *temp, int32_t *base, int st, int size, int sc)
{
    if (i < sc)
        temp[0] = (2 * base[st * i] + base[st * (sc - i)] + base[st * (i + sc)])>>4;
    else if (i + sc < size)
        temp[0] = (2 * base[st * i] + base[st * (i - sc)] + base[st * (i + sc)])>>4;
    else if (i < size)
        temp[0] = (2 * base[st * i] + base[st * (i - sc)] + base[st * (2 * size - 2 - (i + sc))])>>4;
}
/**
    \fn WaveletDenoiseProcess_Core
*/
void ADMVideoWaveletDenoise::WaveletDenoiseProcess_Core(int32_t *buf[4], int levels, unsigned int width, unsigned int height, double amount, double softness)
{
    int32_t thold, tsoft;
    unsigned int i, lev, minDim, sc, lpass, hpass, size, col, row;
    minDim = ((width < height) ? width : height);
    size = width * height;
    hpass = 0;
    int64_t stdev[8];
    unsigned int samples[8];
    uint32_t sqr;
    int tholds[8], tsofts[8], soft;

    soft = std::round(softness * (1 << 8));

    for (lev = 0; lev < levels; lev++)
    {
        sc = (1 << lev);
        if (sc*2 >= minDim) break;
        lpass = ((lev & 1) + 1);
        for (row = 0; row < height; row++)
        {
            WaveletDenoiseProcess_HatTransformHorizontal(buf[lpass] + row * width, buf[hpass] + row * width, width, sc);
        }
        for (row = 0; row < height; row++)
        {
            for (col = 0; col < width; col++)
            {
                WaveletDenoiseProcess_HatTransformVertical(row, buf[3]+ row * width + col, buf[lpass] + col, width, height, sc);
            }
        }
        memcpy(buf[lpass],buf[3],size*sizeof(int32_t));

        thold = std::round((5.0 / (1 << 6) * exp (-2.6 * std::sqrt (lev + 1)) * 0.8002 / exp (-2.6))* (1 << 16));

        for (i=0; i<8; i++)
        {
            stdev[i] = 0;
            samples[i] = 0;
        }

        for (i = 0; i < size; i++)
        {
            buf[hpass][i] -= buf[lpass][i];
            if (buf[hpass][i] < thold && buf[hpass][i] > -thold)
            {
                sqr = buf[hpass][i];    // if negative --> uint will be huge, but huge*huge == negative*negative
                sqr = sqr*sqr;
                stdev[(buf[lpass][i] >> 13) & 0x7] += sqr;
                samples[(buf[lpass][i] >> 13) & 0x7]++;
            }
        }

        for (i = 0; i < 8; i++)
        {
            tholds[i] = std::round(amount * std::sqrt ((double)stdev[i] / (samples[i] + 1)));
            tsofts[i] = std::round((1.0 - softness) * amount * std::sqrt ((double)stdev[i] / (samples[i] + 1)));
        }

        for (i = 0; i < size; i++)
        {
            thold = tholds[(buf[lpass][i] >> 13) & 0x7];
            tsoft = tsofts[(buf[lpass][i] >> 13) & 0x7];

            if (buf[hpass][i] < -thold)
                buf[hpass][i] += tsoft;
            else if (buf[hpass][i] > thold)
                buf[hpass][i] -= tsoft;
            else {
                buf[hpass][i] *= soft;
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
    \fn WaveletDenoiseProcess_C
*/
void ADMVideoWaveletDenoise::WaveletDenoiseProcess_C(ADMImage *img, float threshold, float softness, bool highq, bool chroma)
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

    if (threshold < 0.0) threshold = 0.0;
    if (threshold > 1.0) threshold = 1.0;
    if (softness < 0.0) softness = 0.0;
    if (softness > 1.0) softness = 1.0;

    if(img->_range == ADM_COL_RANGE_MPEG)
    {
        limitL = 16;
        limitH = 235;
    }

    threshold = (threshold*threshold)*10.0;

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
    WaveletDenoiseProcess_Core(buffer, (highq ? 5:3), width, height, threshold, softness);

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

    if (chroma)
    {
        if(img->_range == ADM_COL_RANGE_MPEG)
        {
            limitH = 239;
        }

        for (int p=1; p<3; p++)
        {
            // UV planes
            stride=img->GetPitch((ADM_PLANE)p);
            ptr=img->GetWritePtr((ADM_PLANE)p);
            bufptr = buffer[0];
            for(int y=0;y<(height/2);y++)
            {
                for (int x=0;x<(width/2);x++)
                {
                    pixel = ptr[x];
                    *bufptr = pixel << 8;
                    bufptr++;
                }
                ptr+=stride;
            }

            // Algo
            WaveletDenoiseProcess_Core(buffer, (highq ? 5:3), (width/2), (height/2), threshold, softness);

            // UV planes
            stride=img->GetPitch((ADM_PLANE)p);
            ptr=img->GetWritePtr((ADM_PLANE)p);
            bufptr = buffer[0];
            for(int y=0;y<(height/2);y++)
            {
                for (int x=0;x<(width/2);x++)
                {
                    pixel = *bufptr >> 8;
                    bufptr++;
                    if (pixel < limitL) pixel = limitL;
                    if (pixel > limitH) pixel = limitH;
                    ptr[x] = pixel;
                }
                ptr+=stride;
            }
        }
    }

    free(buf);
}

/**
    \fn configure
*/
bool ADMVideoWaveletDenoise::configure()
{
    uint8_t r=0;

    r=  DIA_getWaveletDenoise(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoWaveletDenoise::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Strength: %.2f, Radius: %.2f, Chroma denoising: %s%s",_param.threshold, _param.softness,(_param.chroma ? "yes":"no"),(_param.highq ? ", High quality":""));
    return s;
}
/**
    \fn ctor
*/
ADMVideoWaveletDenoise::ADMVideoWaveletDenoise(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,waveletDenoise_param,&_param))
        reset(&_param);
    update();
}
/**
    \fn reset
*/
void ADMVideoWaveletDenoise::reset(waveletDenoise *cfg)
{
    cfg->threshold = 0.0;
    cfg->softness = 0.5;
    cfg->highq = true;
    cfg->chroma=false;
}
/**
    \fn valueLimit
*/
float ADMVideoWaveletDenoise::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn valueLimit
*/
int32_t ADMVideoWaveletDenoise::valueLimit(int32_t val, int32_t min, int32_t max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoWaveletDenoise::update(void)
{
    _threshold=valueLimit(_param.threshold,0.0,1.0);
    _softness=valueLimit(_param.softness,0.0,1.0);
    _highq=_param.highq;
    _chroma=_param.chroma;
}
/**
    \fn dtor
*/
ADMVideoWaveletDenoise::~ADMVideoWaveletDenoise()
{
    
}
/**
    \fn getCoupledConf
*/
bool ADMVideoWaveletDenoise::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, waveletDenoise_param,&_param);
}

void ADMVideoWaveletDenoise::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, waveletDenoise_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoWaveletDenoise::getNextFrame(uint32_t *fn,ADMImage *image)
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

    WaveletDenoiseProcess_C(image, _threshold, _softness, _highq, _chroma);

    return 1;
}

