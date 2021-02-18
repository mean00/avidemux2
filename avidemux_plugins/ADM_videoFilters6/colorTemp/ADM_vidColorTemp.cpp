/***************************************************************************
                          colorTemp filter 
    Algorithm:
        Copyright (C) 1999 Winston Chang
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
#include "colorTemp.h"
#include "colorTemp_desc.cpp"
#include "ADM_vidColorTemp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getColorTemp(colorTemp *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoColorTemp,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoColorTemp,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_COLORS,            // Category
                                      "colorTemp",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("colorTemp","Color temperature"),            // Display name
                                      QT_TRANSLATE_NOOP("colorTemp","Adjust color temperature.") // Description
                                  );
/**
    \fn ColorTempProcess_C
*/
void ADMVideoColorTemp::ColorTempProcess_C(ADMImage *img, float temperature, float angle)
{
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int ystride,ustride,vstride;
    uint8_t * yptr, * uptr, * vptr;
    int pixel,max;
    int limitL = 0;
    int limitH = 255;
    bool reducedRange = false;

    angle = angle*M_PI/180.;

    float ushiftf = +100.0*std::cos(angle)*temperature;
    float vshiftf = -100.0*std::sin(angle)*temperature;

    int ushift, vshift;

    if(img->_range == ADM_COL_RANGE_MPEG)
    {
        ushiftf *= 224.0/256.0;
        vshiftf *= 224.0/256.0;
        limitL = 16;
        limitH = 239;
        reducedRange = true;
    }

    // Y plane unchanged

    // UV planes
    ystride=img->GetPitch(PLANAR_Y);
    yptr=img->GetWritePtr(PLANAR_Y);
    ustride=img->GetPitch(PLANAR_U);
    uptr=img->GetWritePtr(PLANAR_U);
    vstride=img->GetPitch(PLANAR_V);
    vptr=img->GetWritePtr(PLANAR_V);
    for(int y=0;y<height/2;y++)	// 4:2:0
    {
        for (int x=0;x<width/2;x++)
        {
            max = yptr[x*2];
            pixel = yptr[x*2 + 1];
            if (pixel > max) max = pixel;
            pixel = yptr[x*2 + ystride];
            if (pixel > max) max = pixel;
            pixel = yptr[x*2 + ystride + 1];
            if (pixel > max) max = pixel;
            if (reducedRange)
            {
                max -= 16.0;
                if (max < 0.0) max = 0.0;
                ushift = (int)(ushiftf * (float)max/219.0);
                vshift = (int)(vshiftf * (float)max/219.0);
            } else {
                ushift = (int)(ushiftf * (float)max/255.0);
                vshift = (int)(vshiftf * (float)max/255.0);
            }

            pixel = uptr[x];
            pixel += ushift;
            if (pixel < limitL) pixel = limitL;
            if (pixel > limitH) pixel = limitH;
            uptr[x] = pixel;

            pixel = vptr[x];
            pixel += vshift;
            if (pixel < limitL) pixel = limitL;
            if (pixel > limitH) pixel = limitH;
            vptr[x] = pixel;
        }
        yptr+=2*ystride;
        uptr+=ustride;
        vptr+=vstride;
    }
}

/**
    \fn configure
*/
bool ADMVideoColorTemp::configure()
{
    uint8_t r=0;

    r=  DIA_getColorTemp(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoColorTemp::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Temperature :%2.2f, Angle: %.0f",_param.temperature, _param.angle);
    return s;
}
/**
    \fn ctor
*/
ADMVideoColorTemp::ADMVideoColorTemp(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,colorTemp_param,&_param))
        reset(&_param);
    update();
}
/**
    \fn reset
*/
void ADMVideoColorTemp::reset(colorTemp *cfg)
{
    cfg->temperature = 0.0;
    cfg->angle = 30.0;
}
/**
    \fn valueLimit
*/
float ADMVideoColorTemp::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn valueLimit
*/
int32_t ADMVideoColorTemp::valueLimit(int32_t val, int32_t min, int32_t max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoColorTemp::update(void)
{
    _temperature=valueLimit(_param.temperature,-1.0,1.0);
    _angle=valueLimit(_param.angle,0.0,180.0);
}
/**
    \fn dtor
*/
ADMVideoColorTemp::~ADMVideoColorTemp()
{
    
}
/**
    \fn getCoupledConf
*/
bool ADMVideoColorTemp::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, colorTemp_param,&_param);
}

void ADMVideoColorTemp::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, colorTemp_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoColorTemp::getNextFrame(uint32_t *fn,ADMImage *image)
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

    ColorTempProcess_C(image, _temperature, _angle);

    return 1;
}

