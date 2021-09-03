/***************************************************************************
                          FadeFromImage filter
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
#include "fadeFromImage.h"
#include "fadeFromImage_desc.cpp"
#include "ADM_vidFadeFromImage.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getFadeFromImage(fadeFromImage *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ADMVideoFadeFromImage,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_TRANSITION,            // Category
                                      "fadeFromImage",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("fadeFromImage","Fade from first image"),            // Display name
                                      QT_TRANSLATE_NOOP("fadeFromImage","Use image at start time for fading.") // Description
                                  );

/**
    \fn FadeFromImageCreateBuffers
*/
void ADMVideoFadeFromImage::FadeFromImageCreateBuffers(int w, int h, fadeFromImage_buffers_t * buffers)
{
    buffers->validImg = false;
    buffers->imgCopy = new ADMImageDefault(w,h);
    // make it green
    uint8_t * wplanes[3];
    int strides[3];
    buffers->imgCopy->GetWritePlanes(wplanes);
    buffers->imgCopy->GetPitches(strides);
    memset(wplanes[0], 128, h*strides[0]);
    memset(wplanes[1], 0, (h/2)*strides[1]);
    memset(wplanes[2], 0, (h/2)*strides[2]);
}
/**
    \fn FadeFromImageDestroyBuffers
*/
void ADMVideoFadeFromImage::FadeFromImageDestroyBuffers(fadeFromImage_buffers_t * buffers)
{
    delete buffers->imgCopy;
}

/**
    \fn FadeFromImageProcess_C
*/
void ADMVideoFadeFromImage::FadeFromImageProcess_C(ADMImage *img, int w, int h,  uint64_t absoluteStartPts, fadeFromImage param, fadeFromImage_buffers_t * buffers)
{
    if (!img || !buffers || !buffers->imgCopy) return;
    uint8_t * line;

    uint32_t startMs = param.startTime;
    uint32_t endMs = param.endTime;

    if (startMs > endMs)
    {
        uint64_t tmp = endMs;
        endMs = startMs;
        startMs = tmp;
    }
    
    uint32_t imgMs = (img->Pts+absoluteStartPts)/1000LL;
    
    if (startMs == endMs)
        return;
    if (imgMs < startMs)
        return;
    if (imgMs >= endMs)
        return;

    if (!buffers->validImg)
    {
        if (imgMs <= startMs+1)	// avoid rounding errors
        {
            buffers->validImg = true;
            buffers->imgCopy->duplicateFull(img);
        }
    }

    double frac = ((double)(imgMs - startMs)) / ((double)(endMs - startMs));
    int dir = param.direction % 4;

    uint8_t * imgPlanes[3];
    uint8_t * copyPlanes[3];
    int imgStrides[3];
    int copyStrides[3];

    img->GetWritePlanes(imgPlanes);
    img->GetPitches(imgStrides);
    buffers->imgCopy->GetWritePlanes(copyPlanes);
    buffers->imgCopy->GetPitches(copyStrides);

    switch (param.effect)
    {
        case 0:		// linear blend
            {
                int imgblend = frac*256.0 + 0.49;
                int copyblend = (1.0-frac)*256.0 + 0.49;
                for (int p=0; p<3; p++)
                {
                    int width = ((p==0) ? w:(w/2));
                    int height = ((p==0) ? h:(h/2));
                    int px;
                    uint8_t * ipl = imgPlanes[p];
                    uint8_t * cpl = copyPlanes[p];
                    for (int y=0; y<height; y++)
                    {
                        for (int x=0; x<width; x++)
                        {
                            px = imgblend*ipl[x] + copyblend*cpl[x];
                            ipl[x] = px / 256;
                        }
                        ipl += imgStrides[p];
                        cpl += copyStrides[p];
                    }
                }
            }
            break;
        case 1:		// slide
            {
                // for now, only vertical and horizontal slide supported
                if (dir==0)  // up
                {
                    int displacement = (frac*h)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=0; y<(height-displacement); y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p], copyPlanes[p]+(y+displacement)*copyStrides[p], width);
                        }
                    }
                }
                else
                if (dir==1)  // right
                {
                    int displacement = (frac*w)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=0; y<height; y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p] + displacement, copyPlanes[p]+y*copyStrides[p], (width-displacement));
                        }
                    }
                }
                else
                if (dir==2)  // down
                {
                    int displacement = (frac*h)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=displacement; y<height; y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p], copyPlanes[p]+(y-displacement)*copyStrides[p], width);
                        }
                    }
                }
                else
                //if (dir==3)  // left
                {
                    int displacement = (frac*w)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=0; y<height; y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p], copyPlanes[p]+y*copyStrides[p]+displacement, (width-displacement));
                        }
                    }
                }
            }
            break;
        case 2:		// wipe
            {
                // for now, only vertical and horizontal wipe supported
                if (dir==0)  // up
                {
                    int displacement = (frac*h)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=0; y<(height-displacement); y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p], copyPlanes[p]+y*copyStrides[p], width);
                        }
                    }
                }
                else
                if (dir==1)  // right
                {
                    int displacement = (frac*w)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=0; y<height; y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p]+displacement, copyPlanes[p]+y*copyStrides[p]+displacement, (width-displacement));
                        }
                    }
                }
                else
                if (dir==2)  // down
                {
                    int displacement = (frac*h)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=displacement; y<height; y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p], copyPlanes[p]+y*copyStrides[p], width);
                        }
                    }
                }
                else
                //if (dir==3)  // left
                {
                    int displacement = (frac*w)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=0; y<height; y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p], copyPlanes[p]+y*copyStrides[p], (width-displacement));
                        }
                    }
                }
            }
            break;
        case 3:		// push
            {
                // for now, only vertical and horizontal slide supported
                if (dir==0)  // up
                {
                    int displacement = (frac*h)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=(height-1); y>=(height-displacement); y--)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p], imgPlanes[p]+(y-(height-displacement))*imgStrides[p], width);
                        }
                        for (int y=0; y<(height-displacement); y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p], copyPlanes[p]+(y+displacement)*copyStrides[p], width);
                        }
                    }
                }
                else
                if (dir==1)  // right
                {
                    int displacement = (frac*w)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=0; y<height; y++)
                        {
                            memmove(imgPlanes[p]+y*imgStrides[p], imgPlanes[p]+y*imgStrides[p] + (width-displacement), displacement);
                        }
                        for (int y=0; y<height; y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p] + displacement, copyPlanes[p]+y*copyStrides[p], (width-displacement));
                        }
                    }
                }
                else
                if (dir==2)  // down
                {
                    int displacement = (frac*h)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=0; y<displacement; y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p], imgPlanes[p]+(y+(height-displacement))*imgStrides[p], width);
                        }
                        for (int y=displacement; y<height; y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p], copyPlanes[p]+(y-displacement)*copyStrides[p], width);
                        }
                    }
                }
                else
                //if (dir==3)  // left
                {
                    int displacement = (frac*w)/2 + 0.49;
                    displacement *= 2;
                    for (int p=0; p<3; p++)
                    {
                        int width = ((p==0) ? w:(w/2));
                        int height = ((p==0) ? h:(h/2));
                        if (p==1)
                            displacement /= 2;

                        for (int y=0; y<height; y++)
                        {
                            memmove(imgPlanes[p]+y*imgStrides[p] + (width-displacement), imgPlanes[p]+y*imgStrides[p], displacement);
                        }
                        for (int y=0; y<height; y++)
                        {
                            memcpy(imgPlanes[p]+y*imgStrides[p], copyPlanes[p]+y*copyStrides[p]+displacement, (width-displacement));
                        }
                    }
                }
            }
            break;
        case 4:		// luma
        case 5:		// inv. luma
            {
                if (param.effect == 5)
                    frac = 1.0-frac;
                
                int threshold;
                if(img->_range == ADM_COL_RANGE_MPEG)
                    threshold = frac*220.0 + 16.49;
                else
                    threshold = frac*256.0 + 0.49;
                uint8_t * ipl = imgPlanes[0];
                uint8_t * cpl = copyPlanes[0];
                for (int y=0; y<h; y++)
                {
                    if (param.effect == 5)
                    {
                        for (int x=0; x<w; x++)
                        {
                            if (cpl[x] < threshold)
                                ipl[x] = cpl[x];
                        }
                    } else {
                        for (int x=0; x<w; x++)
                        {
                            if (cpl[x] > threshold)
                                ipl[x] = cpl[x];
                        }
                    }
                    ipl += imgStrides[0];
                    cpl += copyStrides[0];
                }
                ipl = imgPlanes[0];
                cpl = copyPlanes[0];
                int chromaMix,px;
                for (int y=0; y<h/2; y++)
                {
                    for (int x=0; x<w/2; x++)
                    {
                        if (param.effect == 5)
                        {
                            chromaMix  = (cpl[2*x] < threshold) ? 1:0;
                            chromaMix += (cpl[2*x+1] < threshold) ? 1:0;
                            chromaMix += (cpl[2*x+copyStrides[0]] < threshold) ? 1:0;
                            chromaMix += (cpl[2*x+copyStrides[0]+1] < threshold) ? 1:0;
                        } else {
                            chromaMix  = (cpl[2*x] > threshold) ? 1:0;
                            chromaMix += (cpl[2*x+1] > threshold) ? 1:0;
                            chromaMix += (cpl[2*x+copyStrides[0]] > threshold) ? 1:0;
                            chromaMix += (cpl[2*x+copyStrides[0]+1] > threshold) ? 1:0;
                        }
                        px = (4-chromaMix)*imgPlanes[1][x] + chromaMix*copyPlanes[1][x];
                        imgPlanes[1][x] = px / 4;
                        px = (4-chromaMix)*imgPlanes[2][x] + chromaMix*copyPlanes[2][x];
                        imgPlanes[2][x] = px / 4;
                    }
                    ipl += 2*imgStrides[0];
                    cpl += 2*copyStrides[0];
                    imgPlanes[1] += imgStrides[1];
                    imgPlanes[2] += imgStrides[2];
                    copyPlanes[1] += copyStrides[1];
                    copyPlanes[2] += copyStrides[2];
                }
                
            }
            break;
        case 6:		// static random dissolve
        case 7:		// dynamic random dissolve
            {
                int threshold = frac*255.0 + 0.49;
                uint32_t rng_state;
                uint32_t rn;
                uint64_t rng_product;
                
                if (param.effect == 7)
                    rng_state = img->Pts;
                else
                    rng_state = startMs;
                if (rng_state == 0)
                    rng_state = 123456789;

                for (int p=0; p<3; p++)
                {
                    int width = ((p==0) ? w:(w/2));
                    int height = ((p==0) ? h:(h/2));
                    int px;
                    uint8_t * ipl = imgPlanes[p];
                    uint8_t * cpl = copyPlanes[p];
                    for (int y=0; y<height; y++)
                    {
                        for (int x=0; x<width; x++)
                        {
                            uint64_t rng_product = (uint64_t)rng_state * 48271;
                            rng_state = (rng_product & 0x7fffffff) + (rng_product >> 31);
                            rng_state = (rng_state & 0x7fffffff) + (rng_state >> 31);
                            rn = rng_state & 0xFF;
                            if (rn > threshold)
                                ipl[x] = cpl[x];
                        }
                        ipl += imgStrides[p];
                        cpl += copyStrides[p];
                    }
                }
            }
            break;
        default:
                img->duplicateFull(buffers->imgCopy);
            break;
    }
}

/**
    \fn configure
*/
bool ADMVideoFadeFromImage::configure()
{
    uint8_t r=0;

    r=  DIA_getFadeFromImage(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoFadeFromImage::getConfiguration(void)
{
    static char s[512];
    const char * effect=NULL;
    
    char startTimeStr[128];
    char endTimeStr[128];
    snprintf(startTimeStr,127,"%s",ADM_us2plain(_param.startTime*1000LL));
    snprintf(endTimeStr,127,"%s",ADM_us2plain(_param.endTime*1000LL));

    switch (_param.effect)
    {
        case 0:		// linear blend
            effect = "Linear blend";
            break;
        case 1:		// slide
            effect = "Slide";
            break;
        case 2:		// wipe
            effect = "Wipe";
            break;
        case 3:		// push
            effect = "Push";
            break;
        case 4:		// luma
            effect = "Luma dissolve";
            break;
        case 5:		// inv. luma
            effect = "Inverse luma dissolve";
            break;
        case 6:		// static random dissolve
            effect = "Static random dissolve";
            break;
        case 7:		// dynamic random dissolve
            effect = "Dynamic random dissolve";
            break;
    }

    snprintf(s,511,"%s - %s: %s",startTimeStr,endTimeStr,effect);

    return s;
}
/**
    \fn ctor
*/
ADMVideoFadeFromImage::ADMVideoFadeFromImage(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,fadeFromImage_param,&_param))
    {
        // Default value
        _param.startTime = info.markerA / 1000LL;
        _param.endTime = info.markerB / 1000LL;
        _param.effect = 0;
        _param.direction = 0;
    }
    
    FadeFromImageCreateBuffers(info.width,info.height, &(_buffers));
    update();
}
/**
    \fn update
*/
void ADMVideoFadeFromImage::update(void)
{
}
/**
    \fn dtor
*/
ADMVideoFadeFromImage::~ADMVideoFadeFromImage()
{
    FadeFromImageDestroyBuffers(&(_buffers));
}
/**
    \fn getCoupledConf
*/
bool ADMVideoFadeFromImage::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, fadeFromImage_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoFadeFromImage::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, fadeFromImage_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoFadeFromImage::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(!previousFilter->getNextFrame(fn,image)) return false;

    FadeFromImageProcess_C(image,info.width,info.height,getAbsoluteStartTime(),_param, &(_buffers));
 
    return 1;
}

