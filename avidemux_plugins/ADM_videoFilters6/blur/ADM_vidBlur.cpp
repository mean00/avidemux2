/***************************************************************************
                          Blur filter ported from frei0r
    Algorithm:
        Copyright Mario Klingemann
        Copyright Maxim Shemanarev
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
#include "blur.h"
#include "blur_desc.cpp"
#include "ADM_vidBlur.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getBlur(blur *param, ADM_coreVideoFilter *in);




// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoBlur,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoBlur,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_SHARPNESS,            // Category
                                      "blur",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("blur","Blur"),            // Display name
                                      QT_TRANSLATE_NOOP("blur","Based on Mario Klingemann's stack blur algorithm.") // Description
                                  );


static const uint16_t g_stack_blur8_mul[255] = 
    {
        512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
        454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
        482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
        437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
        497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
        320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
        446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
        329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
        505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
        399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
        324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
        268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
        451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
        385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
        332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
        289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
    };

static const uint8_t g_stack_blur8_shr[255] = 
    {
          9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 
         17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 
         19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
         20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
         21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
         21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 
         23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
    };

/**
    \fn BlurCreateBuffers
*/
void ADMVideoBlur::BlurCreateBuffers(int w, int h, int * rgbBufStride, ADM_byteBuffer ** rgbBufRaw, ADMImageRef ** rgbBufImage, ADMColorScalerFull ** convertYuvToRgb, ADMColorScalerFull ** convertRgbToYuv)
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
    \fn BlurDestroyBuffers
*/
void ADMVideoBlur::BlurDestroyBuffers(ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv)
{
    if (convertYuvToRgb) delete convertYuvToRgb;
    if (convertRgbToYuv) delete convertRgbToYuv;
    if (rgbBufRaw) rgbBufRaw->clean();
    if (rgbBufImage) delete rgbBufImage;
    if (rgbBufRaw) delete rgbBufRaw;
}
/**
    \fn BlurProcess_C
*/
void ADMVideoBlur::BlurProcess_C(ADMImage *img, int w, int h, unsigned int radius, int rgbBufStride, ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv)
{
    if (!img || !rgbBufRaw || !rgbBufImage || !convertYuvToRgb || !convertRgbToYuv) return;
    uint8_t * line;

    if (radius < 1) return;	// nothing to do
    if (radius > 254) radius = 254;


    convertYuvToRgb->convertImage(img,rgbBufRaw->at(0));

    uint_fast32_t div;
    uint_fast32_t mul_sum;
    uint_fast32_t shr_sum;

    div = radius * 2 + 1;
    mul_sum = g_stack_blur8_mul[radius];
    shr_sum = g_stack_blur8_shr[radius];

    uint32_t * stack = (uint32_t *)malloc(div * sizeof(uint32_t));
    if (!stack) return;

    uint_fast32_t x, y, xp, yp, i;
    uint_fast32_t stack_ptr;
    uint_fast32_t stack_start;

    const uint8_t * src_pix_ptr;
          uint8_t * dst_pix_ptr;
    uint8_t *     stack_pix_ptr;

    uint_fast32_t sum_r;
    uint_fast32_t sum_g;
    uint_fast32_t sum_b;
    uint_fast32_t sum_in_r;
    uint_fast32_t sum_in_g;
    uint_fast32_t sum_in_b;
    uint_fast32_t sum_out_r;
    uint_fast32_t sum_out_g;
    uint_fast32_t sum_out_b;

    uint_fast32_t wm  = w - 1;
    uint_fast32_t hm  = h - 1;

    for(y = 0; y < h; y++)
    {
        sum_r = 
        sum_g = 
        sum_b = 
        sum_in_r = 
        sum_in_g = 
        sum_in_b = 
        sum_out_r = 
        sum_out_g = 
        sum_out_b = 0;

        src_pix_ptr = rgbBufRaw->at(y*rgbBufStride+ 4*0);
        for(i = 0; i <= radius; i++)
        {
            stack_pix_ptr    = (uint8_t *)&stack[i];
            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;
            sum_out_r       += src_pix_ptr[0];
            sum_r           += src_pix_ptr[0] * (i + 1);
            sum_out_g       += src_pix_ptr[1];
            sum_g           += src_pix_ptr[1] * (i + 1);
            sum_out_b       += src_pix_ptr[2];
            sum_b           += src_pix_ptr[2] * (i + 1);
        }
        for(i = 1; i <= radius; i++)
        {
            if(i <= wm) src_pix_ptr += sizeof(uint32_t); 
            stack_pix_ptr = (uint8_t *)&stack[i + radius];
            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;
            sum_in_r        += src_pix_ptr[0];
            sum_r           += src_pix_ptr[0] * (radius + 1 - i);
            sum_in_g        += src_pix_ptr[1];
            sum_g           += src_pix_ptr[1] * (radius + 1 - i);
            sum_in_b        += src_pix_ptr[2];
            sum_b           += src_pix_ptr[2] * (radius + 1 - i);
        }

        stack_ptr = radius;
        xp = radius;
        if(xp > wm) xp = wm;
        src_pix_ptr = rgbBufRaw->at(y*rgbBufStride+ 4*xp);
        dst_pix_ptr = rgbBufRaw->at(y*rgbBufStride+ 4*0);
        for(x = 0; x < w; x++)
        {
            dst_pix_ptr[0] = (sum_r * mul_sum) >> shr_sum;
            dst_pix_ptr[1] = (sum_g * mul_sum) >> shr_sum;
            dst_pix_ptr[2] = (sum_b * mul_sum) >> shr_sum;
            dst_pix_ptr   += sizeof(uint32_t);

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;
   
            stack_start = stack_ptr + div - radius;
            if(stack_start >= div) stack_start -= div;
            stack_pix_ptr = (uint8_t *)&stack[stack_start];

            sum_out_r -= stack_pix_ptr[0];
            sum_out_g -= stack_pix_ptr[1];
            sum_out_b -= stack_pix_ptr[2];

            if(xp < wm) 
            {
                src_pix_ptr += sizeof(uint32_t);
                ++xp;
            }

            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;

            sum_in_r += src_pix_ptr[0];
            sum_in_g += src_pix_ptr[1];
            sum_in_b += src_pix_ptr[2];
            sum_r    += sum_in_r;
            sum_g    += sum_in_g;
            sum_b    += sum_in_b;

            ++stack_ptr;
            if(stack_ptr >= div) stack_ptr = 0;
            stack_pix_ptr = (uint8_t *)&stack[stack_ptr];

            sum_in_r  -= stack_pix_ptr[0];
            sum_out_r += stack_pix_ptr[0];
            sum_in_g  -= stack_pix_ptr[1];
            sum_out_g += stack_pix_ptr[1];
            sum_in_b  -= stack_pix_ptr[2];
            sum_out_b += stack_pix_ptr[2];
        }
    }

    for(x = 0; x < w; x++)
    {
        sum_r = 
        sum_g = 
        sum_b = 
        sum_in_r = 
        sum_in_g = 
        sum_in_b = 
        sum_out_r = 
        sum_out_g = 
        sum_out_b = 0;

        src_pix_ptr = rgbBufRaw->at(0*rgbBufStride+ 4*x);
        for(i = 0; i <= radius; i++)
        {
            stack_pix_ptr    = (uint8_t *)&stack[i];
            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;
            sum_out_r       += src_pix_ptr[0];
            sum_r           += src_pix_ptr[0] * (i + 1);
            sum_out_g       += src_pix_ptr[1];
            sum_g           += src_pix_ptr[1] * (i + 1);
            sum_out_b       += src_pix_ptr[2];
            sum_b           += src_pix_ptr[2] * (i + 1);
        }
        for(i = 1; i <= radius; i++)
        {
            if(i <= hm) src_pix_ptr += rgbBufStride;
            stack_pix_ptr = (uint8_t *)&stack[i + radius];
            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;
            sum_in_r        += src_pix_ptr[0];
            sum_r           += src_pix_ptr[0] * (radius + 1 - i);
            sum_in_g        += src_pix_ptr[1];
            sum_g           += src_pix_ptr[1] * (radius + 1 - i);
            sum_in_b        += src_pix_ptr[2];
            sum_b           += src_pix_ptr[2] * (radius + 1 - i);
        }

        stack_ptr = radius;
        yp = radius;
        if(yp > hm) yp = hm;
        src_pix_ptr = rgbBufRaw->at(yp*rgbBufStride+ 4*x);
        dst_pix_ptr = rgbBufRaw->at(0*rgbBufStride+ 4*x);
        for(y = 0; y < h; y++)
        {
            dst_pix_ptr[0] = (sum_r * mul_sum) >> shr_sum;
            dst_pix_ptr[1] = (sum_g * mul_sum) >> shr_sum;
            dst_pix_ptr[2] = (sum_b * mul_sum) >> shr_sum;
            dst_pix_ptr += rgbBufStride;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;
   
            stack_start = stack_ptr + div - radius;
            if(stack_start >= div) stack_start -= div;

            stack_pix_ptr = (uint8_t *)&stack[stack_start];
            sum_out_r -= stack_pix_ptr[0];
            sum_out_g -= stack_pix_ptr[1];
            sum_out_b -= stack_pix_ptr[2];

            if(yp < hm) 
            {
                src_pix_ptr += rgbBufStride;
                ++yp;
            }

            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;

            sum_in_r += src_pix_ptr[0];
            sum_in_g += src_pix_ptr[1];
            sum_in_b += src_pix_ptr[2];
            sum_r    += sum_in_r;
            sum_g    += sum_in_g;
            sum_b    += sum_in_b;

            ++stack_ptr;
            if(stack_ptr >= div) stack_ptr = 0;
            stack_pix_ptr = (uint8_t *)&stack[stack_ptr];

            sum_in_r  -= stack_pix_ptr[0];
            sum_out_r += stack_pix_ptr[0];
            sum_in_g  -= stack_pix_ptr[1];
            sum_out_g += stack_pix_ptr[1];
            sum_in_b  -= stack_pix_ptr[2];
            sum_out_b += stack_pix_ptr[2];
        }
    }

    convertRgbToYuv->convertImage(rgbBufImage,img);

    free(stack);
}

/**
    \fn configure
*/
bool ADMVideoBlur::configure()
{
    uint8_t r=0;

    r=  DIA_getBlur(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoBlur::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255," Radius: %d",_param.radius);
    return s;
}
/**
    \fn ctor
*/
ADMVideoBlur::ADMVideoBlur(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,blur_param,&_param))
    {
        _param.radius = 1;
    }
    BlurCreateBuffers(info.width,info.height, &(_rgbBufStride), &(_rgbBufRaw), &(_rgbBufImage), &(_convertYuvToRgb), &(_convertRgbToYuv));
    update();
}
/**
    \fn update
*/
void ADMVideoBlur::update(void)
{
    _radius=_param.radius;
    if (_radius > 200) _radius = 200;
    if (_radius < 1) _radius = 1;
}
/**
    \fn dtor
*/
ADMVideoBlur::~ADMVideoBlur()
{
    BlurDestroyBuffers(_rgbBufRaw, _rgbBufImage, _convertYuvToRgb, _convertRgbToYuv);
}
/**
    \fn getCoupledConf
*/
bool ADMVideoBlur::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, blur_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoBlur::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, blur_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoBlur::getNextFrame(uint32_t *fn,ADMImage *image)
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

    BlurProcess_C(image,info.width,info.height,_radius,_rgbBufStride,_rgbBufRaw, _rgbBufImage, _convertYuvToRgb, _convertRgbToYuv);
 
    return 1;
}

