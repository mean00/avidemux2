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
                                      QT_TRANSLATE_NOOP("blur","Blur selected area.") // Description
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
    \fn BoxBlurLine_C
*/
void ADMVideoBlur::BoxBlurLine_C(uint8_t * line, int len, int pixPitch, uint32_t * stack, unsigned int radius)
{
    uint_fast32_t div;
    uint_fast32_t mul_sum;
    uint_fast32_t shr_sum;

    uint_fast32_t l, p, i;
    uint_fast32_t stack_ptr;

    const uint8_t * src_pix_ptr;
          uint8_t * dst_pix_ptr;
    uint8_t *     stack_pix_ptr;

    uint_fast32_t sum_r = 0;
    uint_fast32_t sum_g = 0;
    uint_fast32_t sum_b = 0;

    uint_fast32_t lm  = len - 1;

    if ((radius > 0) && (len > 1))
    {
        div = radius * 2 + 1;
        shr_sum = 14;
        mul_sum = (1 << shr_sum)/div;

        for(i = 0; i <= radius; i++)
        {
            if ((radius - i) > lm)
                src_pix_ptr = line + pixPitch*lm;
            else
                src_pix_ptr = line + pixPitch*(radius - i);    // fix flickering at the T/L edges, by reflecting the image backward
            stack_pix_ptr    = (uint8_t *)&stack[i];
            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;
            sum_r       += src_pix_ptr[0];
            sum_g       += src_pix_ptr[1];
            sum_b       += src_pix_ptr[2];
        }
        src_pix_ptr = line;
        for(i = 1; i <= radius; i++)
        {
            if(i <= lm) src_pix_ptr += pixPitch; 
            stack_pix_ptr = (uint8_t *)&stack[i + radius];
            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;
            sum_r        += src_pix_ptr[0];
            sum_g        += src_pix_ptr[1];
            sum_b        += src_pix_ptr[2];
        }

        stack_ptr = 0;
        p = radius;
        if(p > lm) p = lm;
        src_pix_ptr = line + pixPitch*p;
        dst_pix_ptr = line;
        for(l = 0; l < len; l++)
        {
            sum_r += src_pix_ptr[0];
            sum_g += src_pix_ptr[1];
            sum_b += src_pix_ptr[2];

            stack_pix_ptr = (uint8_t *)&stack[stack_ptr];

            sum_r  -= stack_pix_ptr[0];
            sum_g  -= stack_pix_ptr[1];
            sum_b  -= stack_pix_ptr[2];

            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;

            stack_ptr++;
            if(stack_ptr >= div) stack_ptr = 0;

            dst_pix_ptr[0] = (sum_r * mul_sum) >> shr_sum;
            dst_pix_ptr[1] = (sum_g * mul_sum) >> shr_sum;
            dst_pix_ptr[2] = (sum_b * mul_sum) >> shr_sum;
            dst_pix_ptr   += pixPitch;

            if(p < lm) 
                src_pix_ptr += pixPitch;
            else if(p < lm*2) 
                src_pix_ptr -= pixPitch;    // fix flickering at the B/R edges, by reflecting the image backward
            ++p;
        }
    }
}
/**
    \fn StackBlurLine_C
*/
void ADMVideoBlur::StackBlurLine_C(uint8_t * line, int len, int pixPitch, uint32_t * stack, unsigned int radius)
{
    uint_fast32_t div;
    uint_fast32_t mul_sum;
    uint_fast32_t shr_sum;

    uint_fast32_t l, p, i;
    uint_fast32_t stack_ptr;
    uint_fast32_t stack_start;

    const uint8_t * src_pix_ptr;
          uint8_t * dst_pix_ptr;
    uint8_t *     stack_pix_ptr;

    uint_fast32_t sum_r = 0;
    uint_fast32_t sum_g = 0;
    uint_fast32_t sum_b = 0;
    uint_fast32_t sum_in_r = 0;
    uint_fast32_t sum_in_g = 0;
    uint_fast32_t sum_in_b = 0;
    uint_fast32_t sum_out_r = 0;
    uint_fast32_t sum_out_g = 0;
    uint_fast32_t sum_out_b = 0;

    uint_fast32_t lm  = len - 1;

    if ((radius > 0) && (len > 1))
    {
        div = radius * 2 + 1;
        mul_sum = g_stack_blur8_mul[radius];
        shr_sum = g_stack_blur8_shr[radius];

        for(i = 0; i <= radius; i++)
        {
            if ((radius - i) > lm)
                src_pix_ptr = line + pixPitch*lm;
            else
                src_pix_ptr = line + pixPitch*(radius - i);    // fix flickering at the T/L edges, by reflecting the image backward
            stack_pix_ptr    = (uint8_t *)&stack[i];
            *(uint32_t*)stack_pix_ptr = *(uint32_t*)src_pix_ptr;
            sum_out_r       += src_pix_ptr[0];
            sum_r           += src_pix_ptr[0] * (i + 1);
            sum_out_g       += src_pix_ptr[1];
            sum_g           += src_pix_ptr[1] * (i + 1);
            sum_out_b       += src_pix_ptr[2];
            sum_b           += src_pix_ptr[2] * (i + 1);
        }
        src_pix_ptr = line;
        for(i = 1; i <= radius; i++)
        {
            if(i <= lm) src_pix_ptr += pixPitch; 
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
        p = radius;
        if(p > lm) p = lm;
        src_pix_ptr = line + pixPitch*p;
        dst_pix_ptr = line;
        for(l = 0; l < len; l++)
        {
            dst_pix_ptr[0] = (sum_r * mul_sum) >> shr_sum;
            dst_pix_ptr[1] = (sum_g * mul_sum) >> shr_sum;
            dst_pix_ptr[2] = (sum_b * mul_sum) >> shr_sum;
            dst_pix_ptr   += pixPitch;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;
   
            stack_start = stack_ptr + div - radius;
            if(stack_start >= div) stack_start -= div;
            stack_pix_ptr = (uint8_t *)&stack[stack_start];

            sum_out_r -= stack_pix_ptr[0];
            sum_out_g -= stack_pix_ptr[1];
            sum_out_b -= stack_pix_ptr[2];

            if(p < lm) 
                src_pix_ptr += pixPitch;
            else if(p < lm*2) 
                src_pix_ptr -= pixPitch;    // fix flickering at the B/R edges, by reflecting the image backward
            ++p;

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
}
/**
    \fn BlurProcess_C
*/
void ADMVideoBlur::BlurProcess_C(ADMImage *img, int w, int h, blur param, int rgbBufStride, ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv)
{
    if (!img || !rgbBufRaw || !rgbBufImage || !convertYuvToRgb || !convertRgbToYuv) return;
    uint8_t * line;

    unsigned int radius = param.radius;
    if (radius > 254) radius = 254;
    if (radius < 1) return;	// nothing to do

    unsigned int algorithm = param.algorithm;
    if (algorithm > 2) algorithm = 2;

    int left = param.left;
    int right = param.right;
    int top = param.top;
    int bottom = param.bottom;

    if ((left < 0) || (left >= w)) return;
    if ((right < 0) || (right >= w)) return;
    if ((top < 0) || (top >= h)) return;
    if ((bottom < 0) || (bottom >= h)) return;
    if ((left+right) >= w) return;
    if ((top+bottom) >= h) return;

    uint32_t * stack = (uint32_t *)malloc(512 * sizeof(uint32_t));    // max 254*2+1=509 needed
    if (!stack) return;

    convertYuvToRgb->convertImage(img,rgbBufRaw->at(0));

    int x, y;
    uint8_t * rgbPtr = rgbBufRaw->at(0) + left*4 + top*rgbBufStride;
    w -= (left+right);
    h -= (top+bottom);

    if (algorithm == 0) {    // Box blur
        for(y = 0; y < h; y++)
        {
            BoxBlurLine_C((rgbPtr + y*rgbBufStride), w, 4, stack, radius);
        };

        for(x = 0; x < w; x++)
        {
            BoxBlurLine_C((rgbPtr + x*4), h, rgbBufStride, stack, radius);
        };
    } else
    if (algorithm == 1) {    // Gaussian blur
        for(y = 0; y < h; y++)
        {
            StackBlurLine_C((rgbPtr + y*rgbBufStride), w, 4, stack, radius);
        };

        for(x = 0; x < w; x++)
        {
            StackBlurLine_C((rgbPtr + x*4), h, rgbBufStride, stack, radius);
        };
    } else
    if (algorithm == 2) {    // Gaussian blur 2 pass
        radius = (int)((float)radius / 1.4142135623730950488);
        for(y = 0; y < h; y++)
        {
            StackBlurLine_C((rgbPtr + y*rgbBufStride), w, 4, stack, radius);
        };

        for(x = 0; x < w; x++)
        {
            StackBlurLine_C((rgbPtr + x*4), h, rgbBufStride, stack, radius);
        };

        radius++;
        if (radius > 254) radius = 254;
        for(y = 0; y < h; y++)
        {
            StackBlurLine_C((rgbPtr + y*rgbBufStride), w, 4, stack, radius);
        };

        for(x = 0; x < w; x++)
        {
            StackBlurLine_C((rgbPtr + x*4), h, rgbBufStride, stack, radius);
        };
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
    const char * algo=NULL;
    switch(_param.algorithm) {
        default:
        case 0: algo="Box";
            break;
        case 1: algo="Near Gaussian";
            break;
        case 2: algo="Gaussian 2 pass";
            break;
    }
    snprintf(s,255,"%s blur, Radius: %d. Left: %u, right: %u, top: %u, bottom: %u ",algo,_param.radius, (unsigned int)_param.left, (unsigned int)_param.right, (unsigned int)_param.top, (unsigned int)_param.bottom);

    return s;
}
/**
    \fn ctor
*/
ADMVideoBlur::ADMVideoBlur(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,blur_param,&_param))
    {
        // Default value
        _param.left=0;
        _param.right=0;
        _param.top=0;
        _param.bottom=0;
        _param.rubber_is_hidden=false;
        _param.algorithm = 0;
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

    BlurProcess_C(image,info.width,info.height,_param,_rgbBufStride,_rgbBufRaw, _rgbBufImage, _convertYuvToRgb, _convertRgbToYuv);
 
    return 1;
}

