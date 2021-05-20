/***************************************************************************
                          Flip filter
    Algorithm:
        Copyright 2009 by mean
        Copyright 2011 by mean
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
#include "flip.h"
#include "flip_desc.cpp"
#include "ADM_vidFlip.h"

#define ADMVideoFlip_HORIZONTAL_FLIP    0
extern uint8_t DIA_getFlip(flip *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoFlip,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoFlip,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_TRANSFORM,            // Category
                                      "flip",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("flip","Flip"),            // Display name
                                      QT_TRANSLATE_NOOP("flip","Vertically/Horizontally flip the image.") // Description
                                  );

/**
    \fn FlipProcess_C
*/
void ADMVideoFlip::FlipProcess_C(ADMImage *img, uint8_t * scratch, uint32_t flipdir)
{
    if (!img || !scratch) return;
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    uint8_t * plane[3];
    int stride[3];

    img->GetPitches(stride);
    img->GetWritePlanes(plane);
    
    if (flipdir == ADMVideoFlip_HORIZONTAL_FLIP)
    {
        for (int p=0; p<3; p++)
        {
            if (p==1)
            {
                width >>= 1;
                height >>= 1;
            }
            
            uint8_t * line = plane[p];
            
            for (int i=0; i<height; i++)
            {
                int count=width>>1;
                uint8_t *h=line;
                uint8_t *e=line+width-1;
                uint8_t r;
                while(count--)
                {
                     r=*e;
                    *e=*h;
                    *h= r;
                    h++;e--;
                }
                line += stride[p];
            }
        }
    } else {    // Vertical flip
        for (int p=0; p<3; p++)
        {
            if (p==1)
            {
                width >>= 1;
                height >>= 1;
            }
            
            int count=height>>1;
            for(int i=0;i<count;i++)
            {
                uint8_t *top=plane[p]+stride[p]*i;
                uint8_t *bottom=plane[p]+(height-i-1)*stride[p];
                memcpy(scratch, top,width);
                memcpy(top, bottom,width);
                memcpy(bottom,scratch,width);
            }
        }
    }
}

/**
    \fn configure
*/
bool ADMVideoFlip::configure()
{
    uint8_t r=0;

    r=  DIA_getFlip(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoFlip::getConfiguration(void)
{
    if (_param.flipdir == ADMVideoFlip_HORIZONTAL_FLIP)
        return "Horizontal flip.";
    else
        return "Vertical flip.";
}
/**
    \fn ctor
*/
ADMVideoFlip::ADMVideoFlip(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,flip_param,&_param))
        _param.flipdir = ADMVideoFlip_HORIZONTAL_FLIP;
    _scratch = (uint8_t*)malloc(info.width);
    update();
}

/**
    \fn update
*/
void ADMVideoFlip::update(void)
{
    _flipdir=_param.flipdir;
}
/**
    \fn dtor
*/
ADMVideoFlip::~ADMVideoFlip()
{
    free(_scratch);
    _scratch = NULL;
}
/**
    \fn getCoupledConf
*/
bool ADMVideoFlip::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, flip_param,&_param);
}

void ADMVideoFlip::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, flip_param, &_param);
}

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoFlip::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(!previousFilter->getNextFrame(fn,image)) return false;

    FlipProcess_C(image, _scratch, _flipdir);

    return 1;
}

