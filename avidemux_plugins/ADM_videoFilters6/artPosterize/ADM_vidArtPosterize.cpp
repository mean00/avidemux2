/***************************************************************************
                          Posterize filter ported from frei0r
    Algorithm:
        Copyright 2006 Jerry Huxtable
        Copyright 2012 Janne Liljeblad
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
#include "artPosterize.h"
#include "artPosterize_desc.cpp"
#include "ADM_vidArtPosterize.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getArtPosterize(artPosterize *param, ADM_coreVideoFilter *in);




// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoArtPosterize,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoArtPosterize,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_ART,            // Category
                                      "artPosterize",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("artPosterize","Posterize"),            // Display name
                                      QT_TRANSLATE_NOOP("artPosterize","Ported from frei0r.") // Description
                                  );

/**
    \fn ArtPosterizeCreateBuffers
*/
void ADMVideoArtPosterize::ArtPosterizeCreateBuffers(int w, int h, int * rgbBufStride, ADM_byteBuffer ** rgbBufRaw, ADMImageRef ** rgbBufImage, ADMColorScalerFull ** convertYuvToRgb, ADMColorScalerFull ** convertRgbToYuv)
{
    *rgbBufStride = ADM_IMAGE_ALIGN(w * 4);
    *rgbBufRaw = new ADM_byteBuffer();
    if (*rgbBufRaw)
        (*rgbBufRaw)->setSize(*rgbBufStride * h);
    *convertYuvToRgb = new ADMColorScalerFull(ADM_CS_BICUBIC,w,h,w,h,ADM_COLOR_YV12,ADM_COLOR_RGB32A);
    *convertRgbToYuv = new ADMColorScalerFull(ADM_CS_BICUBIC,w,h,w,h,ADM_COLOR_RGB32A,ADM_COLOR_YV12);
    *rgbBufImage = new ADMImageRef(w,h);
    if (*rgbBufImage)
    {
        (*rgbBufImage)->_colorspace = ADM_COLOR_RGB32A;
        (*rgbBufImage)->_planes[0] = (*rgbBufRaw)->at(0);
        (*rgbBufImage)->_planes[1] = (*rgbBufImage)->_planes[2] = NULL;
        (*rgbBufImage)->_planeStride[0] = *rgbBufStride;
        (*rgbBufImage)->_planeStride[1] = (*rgbBufImage)->_planeStride[2] = 0;
    }
}
/**
    \fn ArtPosterizeDestroyBuffers
*/
void ADMVideoArtPosterize::ArtPosterizeDestroyBuffers(ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv)
{
    if (convertYuvToRgb) delete convertYuvToRgb;
    if (convertRgbToYuv) delete convertRgbToYuv;
    if (rgbBufRaw) rgbBufRaw->clean();
    if (rgbBufImage) delete rgbBufImage;
    if (rgbBufRaw) delete rgbBufRaw;
}
/**
    \fn ArtPosterizeProcess_C
*/
void ADMVideoArtPosterize::ArtPosterizeProcess_C(ADMImage *img, int w, int h, unsigned int levels, int rgbBufStride, ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv)
{
    if (!img || !rgbBufRaw || !rgbBufImage || !convertYuvToRgb || !convertRgbToYuv) return;
    uint8_t * line;
    int i;
    uint8_t levels_table[256];


    if (levels < 2) levels = 2;
    if (levels > 255) levels = 255;

    for (i = 0; i < 256; i++)
    {
        levels_table[i] = 255 * (levels*i / 256) / (levels-1);
    }

    convertYuvToRgb->convertImage(img,rgbBufRaw->at(0));

    for(int y=0; y<h; y++) {
        line = rgbBufRaw->at(y*rgbBufStride);
        for(int x=0; x<w; x++) {
            line[x*4 + 0] = levels_table[line[x*4 + 0]];
            line[x*4 + 1] = levels_table[line[x*4 + 1]];
            line[x*4 + 2] = levels_table[line[x*4 + 2]];
        }
    }

    convertRgbToYuv->convertImage(rgbBufImage,img);

}

/**
    \fn configure
*/
bool ADMVideoArtPosterize::configure()
{
    uint8_t r=0;

    r=  DIA_getArtPosterize(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoArtPosterize::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Levels: %d",_param.levels);
    return s;
}
/**
    \fn ctor
*/
ADMVideoArtPosterize::ADMVideoArtPosterize(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,artPosterize_param,&_param))
    {
        _param.levels = 64;
    }
    ArtPosterizeCreateBuffers(info.width,info.height, &(_rgbBufStride), &(_rgbBufRaw), &(_rgbBufImage), &(_convertYuvToRgb), &(_convertRgbToYuv));
    update();
}
/**
    \fn update
*/
void ADMVideoArtPosterize::update(void)
{
    _levels=_param.levels;
    if (_levels > 255) _levels = 255;
    if (_levels < 2) _levels = 2;
}
/**
    \fn dtor
*/
ADMVideoArtPosterize::~ADMVideoArtPosterize()
{
    ArtPosterizeDestroyBuffers(_rgbBufRaw, _rgbBufImage, _convertYuvToRgb, _convertRgbToYuv);
}
/**
    \fn getCoupledConf
*/
bool ADMVideoArtPosterize::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, artPosterize_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoArtPosterize::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, artPosterize_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoArtPosterize::getNextFrame(uint32_t *fn,ADMImage *image)
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

    ArtPosterizeProcess_C(image,info.width,info.height,_levels,_rgbBufStride,_rgbBufRaw, _rgbBufImage, _convertYuvToRgb, _convertRgbToYuv);
 
    return 1;
}

