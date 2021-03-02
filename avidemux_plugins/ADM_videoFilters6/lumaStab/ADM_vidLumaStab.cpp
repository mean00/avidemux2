/***************************************************************************
                          lumaStab filter 
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
#include "lumaStab.h"
#include "lumaStab_desc.cpp"
#include "ADM_vidLumaStab.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getLumaStab(lumaStab *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoLumaStab,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoLumaStab,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_COLORS,            // Category
                                      "lumaStab",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("lumaStab","Luma stabilizer"),            // Display name
                                      QT_TRANSLATE_NOOP("lumaStab","Reduce flickering.") // Description
                                  );
/**
    \fn LumaStabProcess_C
*/
void ADMVideoLumaStab::LumaStabProcess_C(ADMImage *img, uint32_t filterLength, float cbratio, float sceneThreshold, bool chroma, float * yHyst, int * yHystlen, float * prevChromaHist, bool * newScene, float * sceneDiff)
{
    if ((!img) || (!yHyst) || (!yHystlen) || (!prevChromaHist)) return;
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int ystride,ustride,vstride;
    uint8_t * yptr, * uptr, * vptr;
    int pixel,cpmul,pmul,padd;
    int i,j,x,y;
    int limitL = 0;
    int limitH = 255;
    int limitcL = 0;
    int limitcH = 255;
    float avgY;
    bool scdet = false;
    float currChromaHist[64];
    memset(currChromaHist, 0, 64*sizeof(float));

    if (filterLength < 2) filterLength = 2;
    if (filterLength > 256) filterLength = 256;
    if (cbratio < 0.0) cbratio = 0.0;
    if (cbratio > 1.0) cbratio = 1.0;
    if (sceneThreshold < 0.0) sceneThreshold = 0.0;
    if (sceneThreshold > 1.0) sceneThreshold = 1.0;

    if(img->_range == ADM_COL_RANGE_MPEG)
    {
        limitL = 16;
        limitH = 235;
        limitcL = 16;
        limitcH = 239;
    }

    if (*yHystlen == 0) scdet = true;

    avgY = 0.0;
    // Y plane: get average luma
    ystride=img->GetPitch(PLANAR_Y);
    yptr=img->GetWritePtr(PLANAR_Y);
    for(y=0;y<height;y++)
    {
        int linesum=0;
        for (x=0;x<width;x++)
        {
            linesum += yptr[x];
        }
        yptr+=ystride;
        avgY += linesum;
    }
    avgY /= (height*width);    // normalize
    if (avgY < 1.0) scdet = true;

    // UV planes: get histograms
    ustride=img->GetPitch(PLANAR_U);
    uptr=img->GetWritePtr(PLANAR_U);
    vstride=img->GetPitch(PLANAR_V);
    vptr=img->GetWritePtr(PLANAR_V);
    for(y=0;y<height/2;y++)	// 4:2:0
    {
        for (x=0;x<width/2;x++)
        {
            currChromaHist[uptr[x]/8 +  0]++;
            currChromaHist[vptr[x]/8 + 32]++;
        }
        uptr+=ustride;
        vptr+=vstride;
    }

    if (!scdet)
    {
        float sum = 0.0;
        float sum2 = 0.0;
        for(i=0; i<64; i++)
        {
            sum += fabs(currChromaHist[i] - prevChromaHist[i]);
        }
        for(i=0; i<64; i++)
        {
            sum2 += fabs(currChromaHist[i] - prevChromaHist[i+64]);
        }
        if (sum2 > sum) sum = sum2;
        sum /= ((height/2) * (width/2));
        sum=std::sqrt(sum/2);

        if ((sceneThreshold < 1.0) && (sum > sceneThreshold))    // disable detect, if sceneThreshold set to max
            scdet = true;

        if (sceneDiff) *sceneDiff = sum;
    }

    if (scdet)
    {
        memcpy(&prevChromaHist[64], currChromaHist, 64*sizeof(float));
    } else {
        memcpy(&prevChromaHist[64], prevChromaHist, 64*sizeof(float));
    }
    memcpy(prevChromaHist, currChromaHist, 64*sizeof(float));

    if (scdet)
    {
        for(i=0; i<256; i++)
        {
            yHyst[i] = avgY;
        }
        *yHystlen = 1;
        // No change on Y plane
    } else {
        float filtY = 0.0;

        *yHystlen += 1;
        if (*yHystlen > 256) *yHystlen = 256;

        memmove(&yHyst[1], &yHyst[0], 255*sizeof(float));
        yHyst[0] = avgY;

        for (i=0; i<filterLength; i++)
        {
            filtY += yHyst[i];
        }
        filtY /= filterLength;

        cpmul = (filtY/avgY)*256;
        pmul = (((filtY/avgY)-1.0)*(1.0-cbratio)+1.0)*256;
        padd = (filtY-avgY)*cbratio*256;

        // Y plane: apply changes
        yptr=img->GetWritePtr(PLANAR_Y);
        for(y=0;y<height;y++)
        {
            for (x=0;x<width;x++)
            {
                pixel = yptr[x];
                pixel = (pixel*pmul+padd)>>8;
                if (pixel < limitL) pixel = limitL;
                if (pixel > limitH) pixel = limitH;
                yptr[x] = pixel;
            }
            yptr+=ystride;
        }

        if (chroma)
        {
            uptr=img->GetWritePtr(PLANAR_U);
            vptr=img->GetWritePtr(PLANAR_V);
            for(y=0;y<height/2;y++)	// 4:2:0
            {
                for (x=0;x<width/2;x++)
                {
                    pixel = uptr[x];
                    pixel = (((pixel-128)*cpmul)>>8)+128;
                    if (pixel<limitcL) pixel = limitcL; 
                    if (pixel>limitcH) pixel = limitcH; 
                    uptr[x] = pixel;
                    pixel = vptr[x];
                    pixel = (((pixel-128)*cpmul)>>8)+128;
                    if (pixel<limitcL) pixel = limitcL; 
                    if (pixel>limitcH) pixel = limitcH; 
                    vptr[x] = pixel;
                }
                uptr+=ustride;
                vptr+=vstride;
            }
        }
    }

    if (newScene) *newScene= scdet;

}
/**
    \fn configure
*/
bool ADMVideoLumaStab::configure()
{
    uint8_t r=0;

    r=  DIA_getLumaStab(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoLumaStab::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Filter length: %u frames, Contrast/Brightness ratio: %.2f, Scene threshold: %.2f, Apply to chroma: %s",_param.filterLength, _param.cbratio, _param.sceneThreshold, (_param.chroma ? "true":"false"));
    return s;
}
/**
    \fn ctor
*/
ADMVideoLumaStab::ADMVideoLumaStab(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,lumaStab_param,&_param))
        reset(&_param);
    update();
    _yHystlen = 0;
    _yHyst = (float*)malloc(256*sizeof(float));
    memset(_prevChromaHist,0,128*sizeof(float));

}
/**
    \fn reset
*/
void ADMVideoLumaStab::reset(lumaStab *cfg)
{
    cfg->filterLength = 8;
    cfg->cbratio = 0.0;
    cfg->sceneThreshold = 0.5;
    cfg->chroma = false;
}
/**
    \fn valueLimit
*/
float ADMVideoLumaStab::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn valueLimit
*/
int32_t ADMVideoLumaStab::valueLimit(int32_t val, int32_t min, int32_t max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoLumaStab::update(void)
{
    _filterLength=valueLimit(_param.filterLength,2,32);
    _cbratio=valueLimit(_param.cbratio,0.0,1.0);
    _sceneThreshold=valueLimit(_param.sceneThreshold,0.0,1.0);
    _chroma=_param.chroma;
}
/**
    \fn dtor
*/
ADMVideoLumaStab::~ADMVideoLumaStab()
{
    free(_yHyst);
}
/**
    \fn getCoupledConf
*/
bool ADMVideoLumaStab::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, lumaStab_param,&_param);
}

void ADMVideoLumaStab::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, lumaStab_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoLumaStab::getNextFrame(uint32_t *fn,ADMImage *image)
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

    LumaStabProcess_C(image, _filterLength, _cbratio, _sceneThreshold, _chroma, _yHyst, &_yHystlen, _prevChromaHist, NULL, NULL);
    return 1;
}

