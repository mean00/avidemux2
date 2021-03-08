/***************************************************************************
                          Cartoon filter ported from frei0r
    Algorithm:
        Copyright 2003 Dries Pruimboom <dries@irssystems.nl>
        Copyright      Denis Rojo <jaromil@dyne.org>
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
#include "artCartoon.h"
#include "artCartoon_desc.cpp"
#include "ADM_vidArtCartoon.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getArtCartoon(artCartoon *param, ADM_coreVideoFilter *in);




// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoArtCartoon,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoArtCartoon,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_ART,            // Category
                                      "artCartoon",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("artCartoon","Cartoon"),            // Display name
                                      QT_TRANSLATE_NOOP("artCartoon","Ported from frei0r.") // Description
                                  );

/**
    \fn ArtCartoonCreateBuffers
*/
void ADMVideoArtCartoon::ArtCartoonCreateBuffers(int w, int h, int * rgbBufStride, ADM_byteBuffer ** rgbBufRaw, ADMImageRef ** rgbBufImage, ADMColorScalerFull ** convertYuvToRgb, ADMColorScalerFull ** convertRgbToYuv)
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
    \fn ArtCartoonDestroyBuffers
*/
void ADMVideoArtCartoon::ArtCartoonDestroyBuffers(ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv)
{
    if (convertYuvToRgb) delete convertYuvToRgb;
    if (convertRgbToYuv) delete convertRgbToYuv;
    if (rgbBufRaw) rgbBufRaw->clean();
    if (rgbBufImage) delete rgbBufImage;
    if (rgbBufRaw) delete rgbBufRaw;
}
/**
    \fn GMError
*/
int ADMVideoArtCartoon::GMError(int err, uint8_t * p1, uint8_t * p2)
{
    int i,t1,t2,res;
    res = 0;
    for (i=0; i<3; i++)
    {
        t1 = p1[i];
        t2 = p2[i];
        res += (t1-t2)*(t1-t2);
    }
    return (res > err ? res:err);
}
/**
    \fn ArtCartoonProcess_C
*/
void ADMVideoArtCartoon::ArtCartoonProcess_C(ADMImage *img, int w, int h, float threshold, uint32_t scatter, uint32_t color, int rgbBufStride, ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv)
{
    if (!img || !rgbBufRaw || !rgbBufImage || !convertYuvToRgb || !convertRgbToYuv) return;
    uint8_t *p, * q, *qm, *qp;
    int i,x,y,t;
    uint8_t matrix[3][3][4];
    uint8_t levels_table[256];

    if (threshold > 1.0) threshold = 1.0;
    if (threshold < 0.0) threshold = 0.0;
    if (scatter > 16) scatter = 16;
    if (scatter < 1) scatter = 1;
    if (color > 32) color = 32;
    if (color < 2) color = 2;

    int thres = (threshold*threshold*65536);
    int diff = scatter;
    for (i = 0; i < 256; i++)
    {
        levels_table[i] = 255 * (color*i / 256) / (color-1);
    }

    convertYuvToRgb->convertImage(img,rgbBufRaw->at(0));

    for(y=diff; y<(h-diff); y++) {
        p = rgbBufRaw->at(y*rgbBufStride);
        for(x=diff; x<(w-diff); x++) {
            q = p + x*4;
            qm = q - diff*rgbBufStride;
            qp = q + diff*rgbBufStride;

            memcpy(matrix[ 0 ][ 0 ],qm-4*diff,4);
            memcpy(matrix[ 0 ][ 1 ],qm       ,4);
            memcpy(matrix[ 0 ][ 2 ],qm+4*diff,4);
            memcpy(matrix[ 1 ][ 0 ],q -4*diff,4);
            memcpy(matrix[ 1 ][ 2 ],q +4*diff,4);
            memcpy(matrix[ 2 ][ 0 ],qp-4*diff,4);
            memcpy(matrix[ 2 ][ 1 ],qp       ,4);
            memcpy(matrix[ 2 ][ 2 ],qp+4*diff,4);

            t = GMError(0, matrix[ 1 ][ 0 ], matrix[ 1 ][ 2 ]);
            t = GMError(t, matrix[ 0 ][ 1 ], matrix[ 2 ][ 1 ]);
            t = GMError(t, matrix[ 0 ][ 0 ], matrix[ 2 ][ 2 ]);
            t = GMError(t, matrix[ 2 ][ 0 ], matrix[ 0 ][ 2 ]);

            *(q+3) = ((t > thres) ? 0xFF:0x00);    // use alpha channel as work storage (one's complement)
        }
    }

    uint8_t mask;
    for(y=0; y<h; y++)
    {
        p = rgbBufRaw->at(y*rgbBufStride);
        for (x=0; x<w; x++)
        {
            mask = ~(*(p+3));
            *p = mask & levels_table[*p]; p++;
            *p = mask & levels_table[*p]; p++;
            *p = mask & levels_table[*p]; p++;
            *p = 0xFF; p++;
        }
    }

    convertRgbToYuv->convertImage(rgbBufImage,img);
}

/**
    \fn configure
*/
bool ADMVideoArtCartoon::configure()
{
    uint8_t r=0;

    r=  DIA_getArtCartoon(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoArtCartoon::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Threshold: %.2f, Scatter: %d, Color level: %d",_param.threshold, _param.scatter, _param.color);
    return s;
}
/**
    \fn ctor
*/
ADMVideoArtCartoon::ADMVideoArtCartoon(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,artCartoon_param,&_param))
    {
        _param.threshold = 0.5;
        _param.scatter = 3;
        _param.color = 8;
    }
    ArtCartoonCreateBuffers(info.width,info.height, &(_rgbBufStride), &(_rgbBufRaw), &(_rgbBufImage), &(_convertYuvToRgb), &(_convertRgbToYuv));
    update();
}
/**
    \fn update
*/
void ADMVideoArtCartoon::update(void)
{
    _threshold=_param.threshold;
    _scatter=_param.scatter;
    _color=_param.color;

}
/**
    \fn dtor
*/
ADMVideoArtCartoon::~ADMVideoArtCartoon()
{
    ArtCartoonDestroyBuffers(_rgbBufRaw, _rgbBufImage, _convertYuvToRgb, _convertRgbToYuv);
}
/**
    \fn getCoupledConf
*/
bool ADMVideoArtCartoon::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, artCartoon_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoArtCartoon::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, artCartoon_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoArtCartoon::getNextFrame(uint32_t *fn,ADMImage *image)
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

    ArtCartoonProcess_C(image,info.width,info.height,_threshold,_scatter,_color,_rgbBufStride,_rgbBufRaw, _rgbBufImage, _convertYuvToRgb, _convertRgbToYuv);
 
    return 1;
}

