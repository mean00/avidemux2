/***************************************************************************
                          VHS filter 
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
#include "artVHS.h"
#include "artVHS_desc.cpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getArtVHS(artVHS *param, ADM_coreVideoFilter *in);
/**
    \class ADMVideoArtVHS
*/
class  ADMVideoArtVHS:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    artVHS          _param;
    float           _lumaBW;
    float           _chromaBW;
    bool            _lumaNoDelay;
    bool            _chromaNoDelay;
    float           _unSync;
  public:
    ADMVideoArtVHS(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoArtVHS();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

  private:
    float valueLimit(float val, float min, float max);
};



// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoArtVHS,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoArtVHS,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_ART,            // Category
                                      "artVHS",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("artVHS","VHS"),            // Display name
                                      QT_TRANSLATE_NOOP("artVHS","VHS effect. Most authentic at lower resolutions.") // Description
                                  );
/**
    \fn ArtVHSProcess_C
*/
void ArtVHSProcess_C(ADMImage *img, float lumaBW, float chromaBW, float unSync, bool lumaNoDelay, bool chromaNoDelay)
{
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int stride;
    uint8_t * ptr;
    float pixel, smf, smfi;
    float rc;
    int random;
    int rndWalk = 0;
    float hShift;
    int lineShift;

    // 
    lumaBW = std::exp(lumaBW*0.69314) - 1.0;
    lumaBW *= lumaBW;
    if (lumaBW < 0.0001) lumaBW = 0.0001;
    chromaBW = std::exp(chromaBW*0.69314) - 1.0;
    chromaBW *= chromaBW;
    if (chromaBW < 0.0001) chromaBW = 0.0001;

    if(img->_range == ADM_COL_RANGE_MPEG)
        img->expandColorRange();

    // Y plane
    stride=img->GetPitch(PLANAR_Y);
    ptr=img->GetWritePtr(PLANAR_Y);
    smf = lumaBW;
    smfi = (1.0-lumaBW);
    for(int y=0;y<height;y++)
    {
        rc = 0;
        random = 0;
        for (int x=0;x<width;x++)
        {
            if ((ptr[x] > 16) && (ptr[x] < 240))
            {
                if (ptr[x] & 0x04)
                {
                    random += ptr[x]&0x03;
                } else {
                    random -= ptr[x]&0x03;
                }
            }
            pixel = ptr[x];
            rc = smf*pixel + smfi*rc;
            ptr[x] = (uint8_t)std::round(rc);
        }
        if (lumaNoDelay)
        {
            rc = 0;
            for (int x=(width-1);x>=0;x--)
            {
                pixel = ptr[x];
                rc = smf*pixel + smfi*rc;
                ptr[x] = (uint8_t)std::round(rc);
            }
        }
        rndWalk += random;
        hShift = (random & 0x7F);    // [0..127]
        hShift = std::exp(hShift/184.0)-1.0;    // [0..1]
        lineShift = std::round(hShift * unSync);
        if (lineShift > 0)
        {
            if (rndWalk > 0) {
                memmove(ptr, ptr+lineShift, width - lineShift);
                memset(ptr+(width - lineShift),0,lineShift);
            } else {
                memmove(ptr+lineShift, ptr, width - lineShift);
                memset(ptr,0,lineShift);
            }
        }
        ptr+=stride;
    }

    // UV planes
    smf = chromaBW;
    smfi = (1.0-chromaBW);
    for (int p=1; p<3; p++)
    {
        stride=img->GetPitch((ADM_PLANE)p);
        ptr=img->GetWritePtr((ADM_PLANE)p);
        for(int y=0;y<height/2;y++)	// 4:2:0
        {
            rc = 0;
            for (int x=0;x<width/2;x++)
            {
                pixel = ptr[x];
                pixel -= 128;
                rc = smf*pixel + smfi*rc;
                ptr[x] = (uint8_t)std::round( (rc ) + 128);
            }
            if (chromaNoDelay)
            {
                rc = 0;
                for (int x=(width/2 - 1);x>=0;x--)
                {
                    pixel = ptr[x];
                    pixel -= 128;
                    rc = smf*pixel + smfi*rc;
                    ptr[x] = (uint8_t)std::round( (rc ) + 128);
                }
            }
            ptr+=stride;
        }
    }
}

/**
    \fn configure
*/
bool ADMVideoArtVHS::configure()
{
    uint8_t r=0;

    r=  DIA_getArtVHS(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoArtVHS::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," LumaBW: %.2f%s, ChromaBW: %.2f%s, UnSync: %.2f",_param.lumaBW, (_param.lumaNoDelay ? " nodelay" : ""), _param.chromaBW, (_param.chromaNoDelay ? " nodelay" : ""), _param.unSync);
    return s;
}
/**
    \fn ctor
*/
ADMVideoArtVHS::ADMVideoArtVHS(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,artVHS_param,&_param))
    {
        _param.lumaBW = 0.66;
        _param.chromaBW = 0.2;
        _param.lumaNoDelay = true;
        _param.chromaNoDelay = false;
        _param.unSync = 1.0;
    }
    update();
}
/**
    \fn valueLimit
*/
float ADMVideoArtVHS::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoArtVHS::update(void)
{
    _lumaBW=valueLimit(_param.lumaBW, 0.0, 1.0);
    _chromaBW=valueLimit(_param.chromaBW, 0.0, 1.0);
    _lumaNoDelay=_param.lumaNoDelay;
    _chromaNoDelay=_param.chromaNoDelay;
    _unSync=valueLimit(_param.unSync, 0.0, 16.0);
}
/**
    \fn dtor
*/
ADMVideoArtVHS::~ADMVideoArtVHS()
{
    
}
/**
    \fn getCoupledConf
*/
bool ADMVideoArtVHS::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, artVHS_param,&_param);
}

void ADMVideoArtVHS::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, artVHS_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoArtVHS::getNextFrame(uint32_t *fn,ADMImage *image)
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

    ArtVHSProcess_C(image, _lumaBW, _chromaBW, _unSync, _lumaNoDelay, _chromaNoDelay);

    return 1;
}

