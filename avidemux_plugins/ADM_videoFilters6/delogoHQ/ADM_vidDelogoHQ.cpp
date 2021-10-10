/***************************************************************************
                          DelogoHQ filter
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
#include "delogoHQ.h"
#include "delogoHQ_desc.cpp"
#include "ADM_vidDelogoHQ.h"
#include "ADM_imageLoader.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getDelogoHQ(delogoHQ *param, ADM_coreVideoFilter *in);




// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoDelogoHQ,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoDelogoHQ,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_SHARPNESS,            // Category
                                      "delogoHQ",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("delogoHQ","DelogoHQ"),            // Display name
                                      QT_TRANSLATE_NOOP("delogoHQ","Clean up arbitrary shaped logo.") // Description
                                  );

/**
    \fn DelogoHQCreateBuffers
*/
void ADMVideoDelogoHQ::DelogoHQCreateBuffers(int w, int h, int * rgbBufStride, ADM_byteBuffer ** rgbBufRaw, ADMImageRef ** rgbBufImage, ADMColorScalerFull ** convertYuvToRgb, ADMColorScalerFull ** convertRgbToYuv)
{
    *rgbBufStride = ADM_IMAGE_ALIGN(w * 4);
    *rgbBufRaw = new ADM_byteBuffer();
    if (*rgbBufRaw)
        (*rgbBufRaw)->setSize(*rgbBufStride * h * 2);
    *convertYuvToRgb = new ADMColorScalerFull(ADM_CS_BICUBIC,w,h,w,h,ADM_PIXFRMT_YV12,ADM_PIXFRMT_RGB32A);
    *convertRgbToYuv = new ADMColorScalerFull(ADM_CS_BICUBIC,w,h,w,h,ADM_PIXFRMT_RGB32A,ADM_PIXFRMT_YV12);
    *rgbBufImage = new ADMImageRef(w,h);
    if (*rgbBufImage)
    {
        (*rgbBufImage)->_pixfrmt = ADM_PIXFRMT_RGB32A;
        (*rgbBufImage)->_planes[0] = (*rgbBufRaw)->at(0);
        (*rgbBufImage)->_planes[1] = (*rgbBufImage)->_planes[2] = NULL;
        (*rgbBufImage)->_planeStride[0] = *rgbBufStride;
        (*rgbBufImage)->_planeStride[1] = (*rgbBufImage)->_planeStride[2] = 0;
    }
}
/**
    \fn DelogoHQDestroyBuffers
*/
void ADMVideoDelogoHQ::DelogoHQDestroyBuffers(ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv)
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
void ADMVideoDelogoHQ::BoxBlurLine_C(uint8_t * line, int len, int pixPitch, uint32_t * stack, unsigned int radius)
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
    \fn DelogoHQProcess_C
*/
void ADMVideoDelogoHQ::DelogoHQProcess_C(ADMImage *img, int w, int h, int * mask, int * maskHint, uint32_t blur, uint32_t gradient, int rgbBufStride, ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv)
{
    if (!img || !mask || !rgbBufRaw || !rgbBufImage || !convertYuvToRgb || !convertRgbToYuv) return;
    uint8_t *p, * q, *s;
    int i,x,y,t;

    uint32_t * stack = (uint32_t *)malloc(512 * sizeof(uint32_t));    // max 254*2+1=509 needed
    if (!stack) return;

    if (blur > 250) blur = 250;
    if (gradient > 100) gradient = 100;

    int maskMinX = 0;
    int maskMinY = 0;
    int maskMaxX = (w-1);
    int maskMaxY = (h-1);
    if (maskHint)
    {
        for (i=0; i<4; i++)
            if (maskHint[i] < 0) maskHint[i] = 0;

        if (maskHint[0] > (w-1)) maskHint[0] = (w-1);
        if (maskHint[1] > (h-1)) maskHint[1] = (h-1);
        if (maskHint[2] > (w-1)) maskHint[2] = (w-1);
        if (maskHint[3] > (h-1)) maskHint[3] = (h-1);
        maskMinX = maskHint[0];
        maskMinY = maskHint[1];
        maskMaxX = maskHint[2];
        maskMaxY = maskHint[3];
    }

    int mAreaX = (maskMaxX - maskMinX);
    int mAreaY = (maskMaxY - maskMinY);
    if (mAreaX <= 0) return;    // nothing to do
    if (mAreaY <= 0) return;    // nothing to do

    convertYuvToRgb->convertImage(img,rgbBufRaw->at(0));

    int currLevel = 1;
    int radius;
    int nl,rx,ry;
    int r,g,b,ravg,gavg,bavg,nsum,lum;
    radius = 3;
    do {
        nl = 0;
        for(y=maskMinY; y<=maskMaxY; y++)
        {
            p = rgbBufRaw->at(y*rgbBufStride);
            for(x=maskMinX; x<=maskMaxX; x++)
            {
                if (mask[y*w+x] == currLevel)
                {
                    nl++;
                    ravg = gavg = bavg = nsum = 0;
                    for (ry=-radius; ry<=radius; ry++)
                    {
                        if ((y+ry) < 0) continue;
                        if ((y+ry) >= h) continue;
                        s = rgbBufRaw->at((y+ry)*rgbBufStride);
                        for (rx=-radius;rx<=radius; rx++)
                        {
                            if ((x+rx) < 0) continue;
                            if ((x+rx) >= w) continue;
                            if (mask[(y+ry)*w+(x+rx)] < currLevel)
                            {
                                r = *(s+(x+rx)*4+0);
                                g = *(s+(x+rx)*4+1);
                                b = *(s+(x+rx)*4+2);
                                lum = r+g+b;
                                lum = std::sqrt(lum);//= 32;
                                ravg += r * (lum+currLevel-mask[(y+ry)*w+(x+rx)]);
                                gavg += g * (lum+currLevel-mask[(y+ry)*w+(x+rx)]);
                                bavg += b * (lum+currLevel-mask[(y+ry)*w+(x+rx)]);
                                nsum += (lum+currLevel-mask[(y+ry)*w+(x+rx)]);
                            }
                        }
                    }
                    if (nsum > 0)
                    {
                        ravg /= nsum;
                        gavg /= nsum;
                        bavg /= nsum;
                    }
                    q = p + x*4;
                    *(q+0) = ravg;
                    *(q+1) = gavg;
                    *(q+2) = bavg;
                }
            }
        }
        currLevel++;
    } while (nl > 0);

    memcpy(rgbBufRaw->at(h*rgbBufStride), rgbBufRaw->at(0), h*rgbBufStride);

    if (blur > 0)
    {
        //for(y = 0; y < h; y++)
        for(y=maskMinY; y<=maskMaxY; y++)
        {
            //BoxBlurLine_C(rgbBufRaw->at((h+y)*rgbBufStride), w, 4, stack, blur);
            BoxBlurLine_C(rgbBufRaw->at((h+y)*rgbBufStride+4*maskMinX), (maskMaxX-maskMinX), 4, stack, blur);
        }

        //for(x = 0; x < w; x++)
        for(x=maskMinX; x<=maskMaxX; x++)
        {
            //BoxBlurLine_C(rgbBufRaw->at(h*rgbBufStride+x*4), h, rgbBufStride, stack, blur);
            BoxBlurLine_C(rgbBufRaw->at((h+maskMinY)*rgbBufStride+(x)*4), (maskMaxY-maskMinY), rgbBufStride, stack, blur);
        }


        for(y=maskMinY; y<=maskMaxY; y++)
        {
            p = rgbBufRaw->at(y*rgbBufStride);
            s = rgbBufRaw->at((h+y)*rgbBufStride);
            for(x=maskMinX; x<=maskMaxX; x++)
            {
                if (mask[y*w+x] > 0)
                {
                    int a,b,c;
                    //currLevel == max+1

                    c = std::round(256.0-(5.12*gradient*((double)(currLevel - mask[y*w+x]))/currLevel));//std::pow(0.5,mask[y*w+x]*(1.0+gradient)/100.0)*256.0);
                    if (c < 0) c = 0;
                    c = (256-c);

                    a = p[x*4+0];
                    b = s[x*4+0];
                    a *= c;
                    b *= (256-c);
                    a += b;
                    a >>= 8;
                    p[x*4+0] = a;

                    a = p[x*4+1];
                    b = s[x*4+1];
                    a *= c;
                    b *= (256-c);
                    a += b;
                    a >>= 8;
                    p[x*4+1] = a;

                    a = p[x*4+2];
                    b = s[x*4+2];
                    a *= c;
                    b *= (256-c);
                    a += b;
                    a >>= 8;
                    p[x*4+2] = a;

                    /*p[x*4+0] = s[x*4+0];
                    p[x*4+1] = s[x*4+1];
                    p[x*4+2] = s[x*4+2];*/
                }
            }
        }
    }

    convertRgbToYuv->convertImage(rgbBufImage,img);

    free(stack);
}
/**
    \fn DelogoHQPrepareMask_C
*/
void ADMVideoDelogoHQ::DelogoHQPrepareMask_C(int * mask, int * maskHint, int w, int h, ADMImage * maskImage)
{
    if (!mask) return;
    if (!maskHint) return;
    if (!maskImage) return;
    if ((w < 1) || (h < 1)) return;


    int i,x,y;
    int stride;
    uint8_t * ptr;

    stride=maskImage->GetPitch(PLANAR_Y);
    ptr=maskImage->GetWritePtr(PLANAR_Y);

    maskHint[0] = maskHint[1] = maskHint[2] = maskHint[3] = -1; 

    for (y=0; y<h; y++)
    {
        for (x=0; x<w; x++)
        {
            mask[y*w+x] = (ptr[x]> 127) ? -1:0;
        }
        ptr+=stride;
    }

    bool hasNegative;
    int expectedNeighbour = 0;
    do {
        hasNegative = false;
        for (y=0; y<h; y++)
        {
            for (x=0; x<w; x++)
            {
                if (mask[y*w+x] >= 0) continue;
                if ((x-1)>=0)
                    if (mask[y*w+(x-1)] == expectedNeighbour)
                    {
                        mask[y*w+x] = (expectedNeighbour+1);
                        continue;
                    }
                if ((x+1)<w)
                    if (mask[y*w+(x+1)] == expectedNeighbour)
                    {
                        mask[y*w+x] = (expectedNeighbour+1);
                        continue;
                    }
                if ((y-1)>=0)
                    if (mask[(y-1)*w+x] == expectedNeighbour)
                    {
                        mask[y*w+x] = (expectedNeighbour+1);
                        continue;
                    }
                if ((y+1)<h)
                    if (mask[(y+1)*w+x] == expectedNeighbour)
                    {
                        mask[y*w+x] = (expectedNeighbour+1);
                        continue;
                    }
                hasNegative = true;
            }
        }
        expectedNeighbour++;
    } while(hasNegative && (expectedNeighbour < 65536));

    for (y=0; y<h; y++)
    {
        for (x=0; x<w; x++)
        {
            if (mask[y*w+x] > 0)
            {
                if ((maskHint[0] < 0) || (maskHint[0] > x)) maskHint[0] = x;
                if ((maskHint[1] < 0) || (maskHint[1] > y)) maskHint[1] = y;
                if (maskHint[2] < x) maskHint[2] = x;
                if (maskHint[3] < y) maskHint[3] = y;
            }
        }
    }
}
/**
    \fn configure
*/
bool ADMVideoDelogoHQ::configure()
{
    uint8_t r=0;

    r=  DIA_getDelogoHQ(&_param, previousFilter);
    reloadImage();
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/

const char   *ADMVideoDelogoHQ::getConfiguration(void)
{
    static char s[2560];
    snprintf(s,2559," Mask file: %s\nBlur radius: %d, gradient: %d",_param.maskfile.c_str(),_param.blur,_param.gradient);
    return s;
}
/**
    \fn ctor
*/
ADMVideoDelogoHQ::ADMVideoDelogoHQ(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,delogoHQ_param,&_param))
    {
        _param.blur = 0;
        _param.gradient = 0;
    }
    _mask = (int *)malloc(info.width * info.height * sizeof(int));
    DelogoHQCreateBuffers(info.width,info.height, &(_rgbBufStride), &(_rgbBufRaw), &(_rgbBufImage), &(_convertYuvToRgb), &(_convertRgbToYuv));
    reloadImage();
    update();
}
/**
    \fn reloadImage
*/
bool ADMVideoDelogoHQ::reloadImage(void)
{
        if(!_param.maskfile.size())
        {
            return false;
        }
        ADMImage * mask=createImageFromFile(_param.maskfile.c_str());
        if(!mask) return false;

        if ((mask->GetWidth(PLANAR_Y) != info.width) || (mask->GetHeight(PLANAR_Y) != info.height))
            memset(_mask, 0, info.width* info.height);
        else
            DelogoHQPrepareMask_C(_mask, _maskHint, info.width, info.height, mask);

        delete mask;

        return true;
}
/**
    \fn update
*/
void ADMVideoDelogoHQ::update(void)
{
    _blur=_param.blur;
    _gradient=_param.gradient;
}
/**
    \fn dtor
*/
ADMVideoDelogoHQ::~ADMVideoDelogoHQ()
{
    DelogoHQDestroyBuffers(_rgbBufRaw, _rgbBufImage, _convertYuvToRgb, _convertRgbToYuv);
    free(_mask);
}
/**
    \fn getCoupledConf
*/
bool ADMVideoDelogoHQ::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, delogoHQ_param,&_param);
}
/**
    \fn setCoupledConf
*/
void ADMVideoDelogoHQ::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, delogoHQ_param, &_param);
}



/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoDelogoHQ::getNextFrame(uint32_t *fn,ADMImage *image)
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

    DelogoHQProcess_C(image,info.width,info.height,_mask,_maskHint,_blur,_gradient,_rgbBufStride,_rgbBufRaw, _rgbBufImage, _convertYuvToRgb, _convertRgbToYuv);
 
    return 1;
}

