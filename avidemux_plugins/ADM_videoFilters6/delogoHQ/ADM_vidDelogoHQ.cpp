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
void ADMVideoDelogoHQ::DelogoHQCreateBuffers(int w, int h, int * plYuvStride, uint16_t ** plYuvBuf, uint16_t ** toLinLut, uint8_t ** toLumaLut)
{
    *plYuvStride = w*4;
    *plYuvBuf = new uint16_t [w*h*4*3];
    *toLinLut = new uint16_t [256];
    *toLumaLut = new uint8_t [4096];
    for (int i=0; i<256; i++)
    {
        (*toLinLut)[i] = std::pow(i/255.0, 2.2)*65535.0 + 0.49;
    }
    for (int i=0; i<4096; i++)
    {
        (*toLumaLut)[i] = std::pow(i/4095.0, 1.0/2.2)*255.0 + 0.49;
    }
}
/**
    \fn DelogoHQDestroyBuffers
*/
void ADMVideoDelogoHQ::DelogoHQDestroyBuffers(uint16_t * plYuvBuf, uint16_t * toLinLut, uint8_t * toLumaLut)
{
    delete [] plYuvBuf;
    delete [] toLinLut;
    delete [] toLumaLut;
}
/**
    \fn BoxBlurLine_C
*/
void ADMVideoDelogoHQ::BoxBlurLine_C(uint16_t * line, int len, int pixPitch, uint64_t * stack, unsigned int radius)
{
    uint64_t div;
    uint64_t mul_sum;
    #define shr_sum (14)

    uint64_t l, p, i;
    uint64_t stack_ptr;

    const uint16_t * src_pix_ptr;
          uint16_t * dst_pix_ptr;
    uint16_t *     stack_pix_ptr;

    uint64_t sum_r = 0;
    uint64_t sum_g = 0;
    uint64_t sum_b = 0;
    uint64_t mulsum_r,mulsum_g,mulsum_b;

    uint64_t lm  = len - 1;

    if ((radius > 0) && (len > 1))
    {
        div = radius * 2 + 1;
        mul_sum = (1 << shr_sum)/div;

        for(i = 0; i <= radius; i++)
        {
            if ((radius - i) > lm)
                src_pix_ptr = line + pixPitch*lm;
            else
                src_pix_ptr = line + pixPitch*(radius - i);    // fix flickering at the T/L edges, by reflecting the image backward
            stack_pix_ptr    = (uint16_t *)&stack[i];
            *(uint64_t*)stack_pix_ptr = *(uint64_t*)src_pix_ptr;
            sum_r       += src_pix_ptr[0];
            sum_g       += src_pix_ptr[1];
            sum_b       += src_pix_ptr[2];
        }
        src_pix_ptr = line;
        for(i = 1; i <= radius; i++)
        {
            if(i <= lm) src_pix_ptr += pixPitch; 
            stack_pix_ptr = (uint16_t *)&stack[i + radius];
            *(uint64_t*)stack_pix_ptr = *(uint64_t*)src_pix_ptr;
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

            stack_pix_ptr = (uint16_t *)&stack[stack_ptr];

            sum_r  -= stack_pix_ptr[0];
            sum_g  -= stack_pix_ptr[1];
            sum_b  -= stack_pix_ptr[2];

            *(uint64_t*)stack_pix_ptr = *(uint64_t*)src_pix_ptr;

            stack_ptr++;
            if(stack_ptr >= div) stack_ptr = 0;

            mulsum_r = (sum_r * mul_sum);
            mulsum_g = (sum_g * mul_sum);
            mulsum_b = (sum_b * mul_sum);
            dst_pix_ptr[0] = (mulsum_r >> shr_sum) + ((mulsum_r >> (shr_sum-1)) & 1);   // rounding
            dst_pix_ptr[1] = (mulsum_g >> shr_sum) + ((mulsum_g >> (shr_sum-1)) & 1);
            dst_pix_ptr[2] = (mulsum_b >> shr_sum) + ((mulsum_b >> (shr_sum-1)) & 1);
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
void ADMVideoDelogoHQ::DelogoHQProcess_C(ADMImage *img, int w, int h, int * mask, int * maskHint, uint32_t blur, uint32_t gradient, int plYuvStride, uint16_t * plYuvBuf, uint16_t * toLinLut, uint8_t * toLumaLut)
{
    if (!img || !mask || !plYuvBuf || !toLinLut || !toLumaLut) return;
    int i,x,y,t;

    uint64_t * stack = (uint64_t *)malloc(512 * sizeof(uint64_t));    // max 254*2+1=509 needed
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

    uint8_t * Yptr=img->GetWritePtr(PLANAR_Y);
    int Ystride=img->GetPitch(PLANAR_Y);
    uint8_t * Uptr=img->GetWritePtr(PLANAR_U);
    int Ustride=img->GetPitch(PLANAR_U);
    uint8_t * Vptr=img->GetWritePtr(PLANAR_V);
    int Vstride=img->GetPitch(PLANAR_V);
    
    uint16_t * plYuvLine;
    uint8_t * Yline, * Uline, * Vline;
    for (int y=0; y<h; y++)
    {
        plYuvLine = plYuvBuf + y*plYuvStride;
        Yline = Yptr + y*Ystride;
        Uline = Uptr + (y/2)*Ustride;
        Vline = Vptr + (y/2)*Vstride;
        for (int x=0; x<w; x++)
        {
            *plYuvLine = toLinLut[*Yline];
            plYuvLine++;
            *plYuvLine = *Uline;
            plYuvLine++;
            *plYuvLine = *Vline;
            plYuvLine++;
            plYuvLine++;
            Yline++;
            if (x%2)
            {
                Uline++;
                Vline++;
            }
        }
    }

    int currLevel = 1;
    int radius;
    int nl,rx,ry;
    uint16_t *p, * q, *s;
    int64_t sY,sU,sV,sumY,sumU,sumV,sumW,sumCW;
    radius = 3;
    do {
        nl = 0;
        for(y=maskMinY; y<=maskMaxY; y++)
        {
            p = plYuvBuf + y*plYuvStride;
            for(x=maskMinX; x<=maskMaxX; x++)
            {
                if (mask[y*w+x] == currLevel)
                {
                    nl++;
                    sumY = sumU = sumV = sumW = sumCW = 0;
                    for (ry=-radius; ry<=radius; ry++)
                    {
                        if ((y+ry) < 0) continue;
                        if ((y+ry) >= h) continue;
                        s = plYuvBuf + (y+ry)*plYuvStride;
                        for (rx=-radius;rx<=radius; rx++)
                        {
                            if ((x+rx) < 0) continue;
                            if ((x+rx) >= w) continue;
                            if (mask[(y+ry)*w+(x+rx)] < currLevel)
                            {
                                sY = *(s+(x+rx)*4+0);
                                if (sY == 0) sY = 1;
                                sU = *(s+(x+rx)*4+1);
                                sV = *(s+(x+rx)*4+2);
                                sumY += sY * (currLevel-mask[(y+ry)*w+(x+rx)]);
                                sumU += sU * (currLevel-mask[(y+ry)*w+(x+rx)]) * sY;
                                sumV += sV * (currLevel-mask[(y+ry)*w+(x+rx)]) * sY;
                                sumW += (currLevel-mask[(y+ry)*w+(x+rx)]);
                                sumCW += (currLevel-mask[(y+ry)*w+(x+rx)]) * sY;
                                //sumW += (lum+currLevel-mask[(y+ry)*w+(x+rx)]);
                            }
                        }
                    }
                    if (sumW > 0)
                    {
                        lldiv_t res;
                        res = lldiv(sumY,sumW); // slightly faster than doing division and modulo separately
                        sumY = res.quot + ((res.rem > sumW/2) ? 1:0); // rounding
                        res = lldiv(sumU,sumCW);
                        sumU = res.quot + ((res.rem > sumCW/2) ? 1:0);
                        res = lldiv(sumV,sumCW);
                        sumV = res.quot + ((res.rem > sumCW/2) ? 1:0);
                    }
                    q = p + x*4;
                    *(q+0) = sumY;
                    *(q+1) = sumU;
                    *(q+2) = sumV;
                }
            }
        }
        currLevel++;
    } while (nl > 0);

    //now currLevel == max+1
    currLevel -= 1;
    
    if ((blur > 0) && (currLevel > 0))
    {
        if (gradient == 0)
        {
            memcpy(plYuvBuf + h*plYuvStride, plYuvBuf, h*plYuvStride*sizeof(uint16_t));
            for(y=maskMinY; y<=maskMaxY; y++)
                BoxBlurLine_C(plYuvBuf + (h+y)*plYuvStride + 4*maskMinX, (maskMaxX-maskMinX), 4, stack, blur);

            for(x=maskMinX; x<=maskMaxX; x++)
                BoxBlurLine_C(plYuvBuf + (h+maskMinY)*plYuvStride + x*4, (maskMaxY-maskMinY), plYuvStride, stack, blur);

            for(y=maskMinY; y<=maskMaxY; y++)
            {
                p = plYuvBuf + y*plYuvStride;
                s = plYuvBuf + (h+y)*plYuvStride;
                for(x=maskMinX; x<=maskMaxX; x++)
                {
                    if (mask[y*w+x] > 0)
                        memcpy(p+4*x, s+4*x, 4*sizeof(uint16_t));
                }
            }
            
        }
        else    // gradient > 0
        {
            int lastBlur = -1;
            double fracBlur = blur;
            double fracBlur2 = blur;
            fracBlur2 /= 2;
            int currBlur;
            double step,step2;
            if (currLevel > 0)
            {
                step = 2.0*(gradient/100.0) * (double)blur / (double)currLevel;
                step2 = (gradient/100.0) * (double)blur / (2.0*currLevel);
            }
            memcpy(plYuvBuf + h*plYuvStride, plYuvBuf, h*plYuvStride*sizeof(uint16_t));
            while (currLevel > 0)
            {
                currBlur = std::round((fracBlur > fracBlur2) ? fracBlur:fracBlur2);
                if (currBlur < 0)
                    currBlur = 0;
                if (currBlur > 250)
                    currBlur = 250;
                if (lastBlur != currBlur)
                {
                    lastBlur = currBlur;
                    if (currBlur > 0)
                    {
                        memcpy(plYuvBuf + 2*h*plYuvStride, plYuvBuf + h*plYuvStride, h*plYuvStride*sizeof(uint16_t));
                        for(y=maskMinY; y<=maskMaxY; y++)
                            BoxBlurLine_C(plYuvBuf + (2*h+y)*plYuvStride + 4*maskMinX, (maskMaxX-maskMinX), 4, stack, currBlur);

                        for(x=maskMinX; x<=maskMaxX; x++)
                            BoxBlurLine_C(plYuvBuf + (2*h+maskMinY)*plYuvStride + x*4, (maskMaxY-maskMinY), plYuvStride, stack, currBlur);
                    }
                }
                if (currBlur > 0)
                {
                    for(y=maskMinY; y<=maskMaxY; y++)
                    {
                        p = plYuvBuf + y*plYuvStride;
                        s = plYuvBuf + (2*h+y)*plYuvStride;
                        for(x=maskMinX; x<=maskMaxX; x++)
                        {
                            if (mask[y*w+x] == currLevel)
                                memcpy(p+4*x, s+4*x, 4*sizeof(uint16_t));
                        }
                    }
                }
                fracBlur -= step;
                fracBlur2 -= step2;
                currLevel -= 1;
            }
        }
    }

    for (int y=0; y<h; y++)
    {
        plYuvLine = plYuvBuf + y*plYuvStride;
        Yline = Yptr + y*Ystride;
        for (int x=0; x<w; x++)
        {
            *Yline = toLumaLut[*plYuvLine >> 4];
            plYuvLine += 4;
            Yline++;
        }
    }
    
    uint16_t * plYuvLineNext;
    uint32_t avgU, avgV;
    for (int y=0; y<h/2; y++)
    {
        Uline = Uptr + y*Ustride;
        Vline = Vptr + y*Vstride;
        plYuvLine = plYuvBuf + (y*2)*plYuvStride;
        plYuvLineNext = plYuvBuf + (y*2+1)*plYuvStride;
        for (int x=0; x<w/2; x++)
        {
            avgU  = plYuvLine[1];
            avgU += plYuvLineNext[1];
            avgU += plYuvLine[1+4];
            avgU += plYuvLineNext[1+4];
            avgV  = plYuvLine[2];
            avgV += plYuvLineNext[2];
            avgV += plYuvLine[2+4];
            avgV += plYuvLineNext[2+4];
            plYuvLine += 8;
            plYuvLineNext += 8;
            avgU /= 4;
            avgV /= 4;
            if (avgU > 255)
                avgU = 255;
            if (avgV > 255)
                avgV = 255;
            *Uline = avgU;
            *Vline = avgV;
            Uline++;
            Vline++;
        }
        
    }
    

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
    DelogoHQCreateBuffers(info.width,info.height, &(_plYuvStride), &(_plYuvBuf), &(_toLinLut), &(_toLumaLut));
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
    DelogoHQDestroyBuffers(_plYuvBuf, _toLinLut, _toLumaLut);
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

    DelogoHQProcess_C(image,info.width,info.height,_mask,_maskHint,_blur,_gradient,_plYuvStride,_plYuvBuf, _toLinLut, _toLumaLut);
 
    return 1;
}

