/***************************************************************************
                          Deband filter 
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
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "deband.h"
#include "deband_desc.cpp"
#include "ADM_vidDeband.h"

extern uint8_t DIA_getDeband(deband *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoDeband,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoDeband,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_COLORS,            // Category
                                      "deband",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("deband","Deband"),            // Display name
                                      QT_TRANSLATE_NOOP("deband","Reduce banding artifacts.") // Description
                                  );
/**
    \fn DebandProcess_C
*/
void ADMVideoDeband::DebandProcess_C(ADMImage *img, ADMImage *tmp, uint32_t range, uint32_t lthresh, uint32_t cthresh)
{
    if (!img || !tmp) return;
    
    tmp->duplicate(img);
    
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    uint8_t * plane[3], * tplane[3];
    int stride[3], tstride[3];
    int p,thr;
    uint32_t rng_state;
    uint32_t rn;
    uint64_t rng_product;


    if (range < 1)
        range = 1;
    if (range > 100)
        range = 100;

    img->GetPitches(stride);
    img->GetWritePlanes(plane);
    tmp->GetPitches(tstride);
    tmp->GetWritePlanes(tplane);

    rng_state = img->Pts;
    if (rng_state == 0)
        rng_state = 123456789;

    thr = lthresh;
    for (p=0; p<3; p++)
    {
        if (p==1)
        {
            width /= 2;
            height /= 2;
            range /= 2;
            if (range < 1)
                range = 1;
            thr = cthresh;
        }
        
        if (thr == 0)
            continue;

        int dx,dy,dxn,dyn, pix, avg;
        int rx=1,ry=1;
        bool chkx,chky,tick=false;

        for (int y = 0; y < height; y++)
        {
            chky = (y < range) || (y >= (height-range));
            for (int x = 0; x < width; x++)
            {
                chkx = (x < range) || (x >= (width-range));
                
                if (range > 1)
                {
                    rng_product = (uint64_t)rng_state * 48271;
                    rng_state = (rng_product & 0x7fffffff) + (rng_product >> 31);
                    rng_state = (rng_state & 0x7fffffff) + (rng_state >> 31);
                    rn = rng_state & 0x00FF;
                    rn *= range;
                    rn >>= 8;
                    if (tick)
                        rx = rn+1;
                    else
                        ry = rn+1;
                    tick = !tick;
                }
                
                dx = x + rx;
                dy = y + ry;
                dxn = x - rx;
                dyn = y - ry;
                if (chkx || chky)
                {
                    if (dx < 0)
                        dx = 0;
                    if (dx >= width)
                        dx = width;
                    if (dy < 0)
                        dy = 0;
                    if (dy >= height)
                        dy = height;
                    if (dxn < 0)
                        dxn = 0;
                    if (dxn >= width)
                        dxn = width;
                    if (dyn < 0)
                        dyn = 0;
                    if (dyn >= height)
                        dyn = height;
                }

                avg  = tplane[p][dxn +  dyn* tstride[p]];
                avg += tplane[p][dx  +  dyn* tstride[p]];
                pix  = tplane[p][x +  y*tstride[p]];
                avg += tplane[p][dxn +  dy * tstride[p]];
                avg += tplane[p][dx  +  dy * tstride[p]];
                pix *= 4;
                if (abs(avg - pix) < thr)
                    plane[p][x + y*stride[p]] = avg/4;
            }
        }
    }
}

/**
    \fn configure
*/
bool ADMVideoDeband::configure()
{
    uint8_t r=0;

    r=  DIA_getDeband(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoDeband::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255,"Range: %u; Thresholds: %u, %u", _param.range, _param.lumaThreshold, _param.chromaThreshold);
    return s;
}
/**
    \fn ctor
*/
ADMVideoDeband::ADMVideoDeband(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,deband_param,&_param))
        reset(&_param);
    work=new ADMImageDefault(info.width,info.height);
    update();
}
/**
    \fn reset
*/
void ADMVideoDeband::reset(deband *cfg)
{
    cfg->range = 16;
    cfg->lumaThreshold = 10;
    cfg->chromaThreshold = 10;
}

/**
    \fn valueLimit
*/
uint32_t ADMVideoDeband::valueLimit(uint32_t val, uint32_t min, uint32_t max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoDeband::update(void)
{
    _range=valueLimit(_param.range,1,100);
    _lthresh=valueLimit(_param.lumaThreshold,0,100);
    _ctresh=valueLimit(_param.chromaThreshold,0,100);
}
/**
    \fn dtor
*/
ADMVideoDeband::~ADMVideoDeband()
{
    if(work) delete work;
    work=NULL;
}
/**
    \fn getCoupledConf
*/
bool ADMVideoDeband::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, deband_param,&_param);
}

void ADMVideoDeband::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, deband_param, &_param);
}

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoDeband::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(!previousFilter->getNextFrame(fn,image)) return false;

    DebandProcess_C(image, work,_range, _lthresh, _ctresh);

    return 1;
}

