/***************************************************************************
                          Mirror filter 
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
#include "artMirror.h"
#include "artMirror_desc.cpp"
#include "ADM_vidArtMirror.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getArtMirror(artMirror *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoArtMirror,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoArtMirror,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_ART,            // Category
                                      "artMirror",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("artMirror","Mirror"),            // Display name
                                      QT_TRANSLATE_NOOP("artMirror","Mirror horizontally or vertically.") // Description
                                  );
/**
    \fn ArtMirrorProcess_C
*/
void ADMVideoArtMirror::ArtMirrorProcess_C(ADMImage *img, uint32_t method, float displacement)
{
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int stride[3];
    uint8_t * ptr[3];
    uint8_t * src, * dst;
    int displ;
    int x,y;
    float pixel;
/*case 0	"Horizontal, Keep left, ";
case 1	"Horizontal, Keep right, ";
case 2	"Vertical, Keep top, ";
case 3	"Vertical, Keep bottom, ";*/

    img->GetPitches(stride);
    img->GetWritePlanes(ptr);

    if (method & 0x2)    // Vertical
    {
        displ = (int)std::round((displacement * (float)height)/2.0);
        displ = valueLimit(displ, 0, height/2);
        displ = (displ >> 1) << 1;    // must be even

        if (method & 0x1)    // Keep bottom
        {
            if (displ > 0)    // apply displacement
            {
                for(y=(height-1);y>=(height/2);y--)
                {
                    memcpy(ptr[0]+y*stride[0], ptr[0]+(y-displ)*stride[0], width);
                }
                for(y=(height/2-1);y>=(height/4);y--)
                {
                    memcpy(ptr[1]+y*stride[1], ptr[1]+(y-displ/2)*stride[1], (width/2));
                    memcpy(ptr[2]+y*stride[2], ptr[2]+(y-displ/2)*stride[2], (width/2));
                }
            }
            for (y=0; y<(height/2);y++) {
                memcpy(ptr[0]+y*stride[0], ptr[0]+((height-1)-y)*stride[0], width);
            }
            for (y=0; y<(height/4);y++) {
                memcpy(ptr[1]+y*stride[1], ptr[1]+((height/2-1)-y)*stride[1], (width/2));
                memcpy(ptr[2]+y*stride[2], ptr[2]+((height/2-1)-y)*stride[2], (width/2));
            }
        } else {    // Keep top
            if (displ > 0)    // apply displacement
            {
                for(y=0;y<(height/2);y++)
                {
                    memcpy(ptr[0]+y*stride[0], ptr[0]+(y+displ)*stride[0], width);
                }
                for(y=0;y<(height/4);y++)
                {
                    memcpy(ptr[1]+y*stride[1], ptr[1]+(y+displ/2)*stride[1], (width/2));
                    memcpy(ptr[2]+y*stride[2], ptr[2]+(y+displ/2)*stride[2], (width/2));
                }
            }
            for (y=(height/2); y<height;y++) {
                memcpy(ptr[0]+y*stride[0], ptr[0]+((height-1)-y)*stride[0], width);
            }
            for (y=(height/4); y<(height/2);y++) {
                memcpy(ptr[1]+y*stride[1], ptr[1]+((height/2-1)-y)*stride[1], (width/2));
                memcpy(ptr[2]+y*stride[2], ptr[2]+((height/2-1)-y)*stride[2], (width/2));
            }
        } 
    } else {    // Horizontal
        displ = (int)std::round((displacement * (float)width)/2.0);
        displ = valueLimit(displ, 0, width/2);
        displ = (displ >> 1) << 1;    // must be even

        if (method & 0x1)    // Keep right
        {
            if (displ > 0)    // apply displacement
            {
                for(y=0;y<height;y++)
                {
                    memmove(ptr[0]+y*stride[0]+width/2, ptr[0]+y*stride[0]+(width/2-displ), (width/2));
                }
                for(y=0;y<(height/2);y++)
                {
                    memmove(ptr[1]+y*stride[1]+width/4, ptr[1]+y*stride[1]+(width/4-displ/2), (width/4));
                    memmove(ptr[2]+y*stride[2]+width/4, ptr[2]+y*stride[2]+(width/4-displ/2), (width/4));
                }
            }
            for(y=0;y<height;y++)
            {
                src = ptr[0]+y*stride[0]+(width/2);
                dst = ptr[0]+y*stride[0]+(width/2)-1;
                for (x=0; x<(width/2); x++)
                {
                    *dst = *src;
                    dst--; src++;
                }
            }
            for(y=0;y<(height/2);y++)
            {
                src = ptr[1]+y*stride[1]+(width/4);
                dst = ptr[1]+y*stride[1]+(width/4)-1;
                for (x=0; x<(width/4); x++)
                {
                    *dst = *src;
                    dst--; src++;
                }
                src = ptr[2]+y*stride[2]+(width/4);
                dst = ptr[2]+y*stride[2]+(width/4)-1;
                for (x=0; x<(width/4); x++)
                {
                    *dst = *src;
                    dst--; src++;
                }
            }
        } else {    // Keep left
            {
                for(y=0;y<height;y++)
                {
                    memmove(ptr[0]+y*stride[0], ptr[0]+y*stride[0]+displ, (width/2));
                }
                for(y=0;y<(height/2);y++)
                {
                    memmove(ptr[1]+y*stride[1], ptr[1]+y*stride[1]+displ/2, (width/4));
                    memmove(ptr[2]+y*stride[2], ptr[2]+y*stride[2]+displ/2, (width/4));
                }
            }
            for(y=0;y<height;y++)
            {
                src = ptr[0]+y*stride[0];
                dst = ptr[0]+y*stride[0]+(width-1);
                for (x=0; x<(width/2); x++)
                {
                    *dst = *src;
                    dst--; src++;
                }
            }
            for(y=0;y<(height/2);y++)
            {
                src = ptr[1]+y*stride[1];
                dst = ptr[1]+y*stride[1]+(width/2-1);
                for (x=0; x<(width/4); x++)
                {
                    *dst = *src;
                    dst--; src++;
                }
                src = ptr[2]+y*stride[2];
                dst = ptr[2]+y*stride[2]+(width/2-1);
                for (x=0; x<(width/4); x++)
                {
                    *dst = *src;
                    dst--; src++;
                }
            }
        } 
    }

}

/**
    \fn configure
*/
bool ADMVideoArtMirror::configure()
{
    uint8_t r=0;

    r=  DIA_getArtMirror(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoArtMirror::getConfiguration(void)
{
    static char s[256];
    const char *m;
    switch(_param.method)
    {
        case 0:
                m = "Horizontal, Keep left, ";
            break;
        case 1:
                m = "Horizontal, Keep right, ";
            break;
        case 2:
                m = "Vertical, Keep top, ";
            break;
        default:
        case 3:
                m = "Vertical, Keep bottom, ";
            break;
    }
    snprintf(s,255,"%s Displacement: %.2f",m,_param.displacement);
    return s;
}
/**
    \fn ctor
*/
ADMVideoArtMirror::ADMVideoArtMirror(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,artMirror_param,&_param))
    {
        _param.method = 0;
        _param.displacement = 0.0;
    }
    update();
}
/**
    \fn valueLimit
*/
float ADMVideoArtMirror::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn valueLimit
*/
int32_t ADMVideoArtMirror::valueLimit(int32_t val, int32_t min, int32_t max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoArtMirror::update(void)
{
    _method=_param.method;
    if (_method > 3) _method = 3;
    _displacement=valueLimit(_param.displacement,0.0,1.0);
}
/**
    \fn dtor
*/
ADMVideoArtMirror::~ADMVideoArtMirror()
{
    
}
/**
    \fn getCoupledConf
*/
bool ADMVideoArtMirror::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, artMirror_param,&_param);
}

void ADMVideoArtMirror::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, artMirror_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoArtMirror::getNextFrame(uint32_t *fn,ADMImage *image)
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

    ArtMirrorProcess_C(image, _method, _displacement);

    return 1;
}

