/***************************************************************************
                          ColorBalance filter 
    Algorithm:
        Copyright 2009 Maksim Golovkin (m4ks1k@gmail.com)
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
#include "colorBalance.h"
#include "colorBalance_desc.cpp"
#include "ADM_vidColorBalance.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getColorBalance(colorBalance *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoColorBalance,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoColorBalance,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_COLORS,            // Category
                                      "colorBalance",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("colorBalance","Color balance"),            // Display name
                                      QT_TRANSLATE_NOOP("colorBalance","Adjust shadow, midtone and highlight.") // Description
                                  );

/**
    \fn gaussSLESolve
*/
void ADMVideoColorBalance::gaussSLESolve(size_t size, double* A, double *solution)
{
    int extSize = size + 1;
    //direct way: tranform matrix A to triangular form
    for(int row = 0; row < size; row++) {
        int col = row;
        int lastRowToSwap = size - 1;
        while (A[row * extSize + col] == 0 && lastRowToSwap > row) { //checking if current and lower rows can be swapped
            for(int i = 0; i < extSize; i++) {
                double tmp = A[row * extSize + i];
                A[row * extSize + i] = A[lastRowToSwap * extSize + i];
                A[lastRowToSwap * extSize + i] = tmp;
            }
            lastRowToSwap--;
        }
        double coeff = A[row * extSize + col];
        for(int j = 0; j < extSize; j++)  
            A[row * extSize + j] /= coeff;
        if (lastRowToSwap > row) {
            for(int i = row + 1; i < size; i++) {
                double rowCoeff = -A[i * extSize + col];
                for(int j = col; j < extSize; j++)
                    A[i * extSize + j] += A[row * extSize + j] * rowCoeff;
            }
        }
    }
    //backward way: find solution from last to first
    memset(solution, 0, size*sizeof(double));
    for(int i = size - 1; i >= 0; i--) {
        solution[i] = A[i * extSize + size];// 
        for(int j = size - 1; j > i; j--) {
            solution[i] -= solution[j] * A[i * extSize + j];
        }
    }
}
/**
    \fn calcParabolaCoeffs
*/
void ADMVideoColorBalance::calcParabolaCoeffs(double* points, double *coeffs)
{
    double m[12];
    for(int i = 0; i < 3; i++) {
        int offset = i * 2;
        m[i * 4] = points[offset] * points[offset];
        m[i * 4 + 1] = points[offset];
        m[i * 4 + 2] = 1;
        m[i * 4 + 3] = points[offset + 1];
    }
    gaussSLESolve(3, m, coeffs);
}
/**
    \fn parabola
*/
float ADMVideoColorBalance::parabola(double x, double* coeffs)
{
    return (float)((coeffs[0] * x + coeffs[1]) * x + coeffs[2]);
}
/**
    \fn quadraticCurve
*/
void ADMVideoColorBalance::quadraticCurve(int * map, float loPoint, float mdPoint, float hiPoint, float curveMin, float curveMax, bool limitedRange, float frMul, float lrMul, float lrOffset)
{
    double points[6], coeffs[3];
    points[0] = 0.0;
    points[1] = loPoint;
    points[2] = 0.5;
    points[3] = mdPoint;
    points[4] = 1.0;
    points[5] = hiPoint;
    calcParabolaCoeffs(points, coeffs);

    if (limitedRange)
    {
        for(int i = 0; i < 220; i++)
        {
            map[i+16] = std::round(valueLimit(parabola(i / 219., coeffs), curveMin, curveMax) * lrMul + lrOffset);
        }
        for(int i = 0; i < 16; i++)
        {
            map[i] = map[16];
        }
        for(int i = 236; i < 256; i++)
        {
            map[i] = map[235];
        }
    } else {
        for(int i = 0; i < 256; i++)
        {
            map[i] = std::round(valueLimit(parabola(i / 255., coeffs), curveMin, curveMax) * frMul);
        }
    }
}
/**
    \fn ColorBalanceProcess_C
*/
void ADMVideoColorBalance::ColorBalanceProcess_C(ADMImage *img, colorBalance cfg)
{
    if (!img) return;
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    uint8_t * ptr, * ptrU, *ptrV, * plane[3];
    int stride[3];
    int limitL = 0;
    int limitH = 255;
    int limitcL = 0;
    int limitcH = 255;

    int * buf = (int*)malloc(256*4*sizeof(int));
    if (!buf) return;
    int * mapY = &buf[0*256];
    int * mapU = &buf[1*256];
    int * mapV = &buf[2*256];
    int * mapS = &buf[3*256];

    img->GetReadPlanes(plane);
    img->GetPitches(stride);

    if(img->_range == ADM_COL_RANGE_MPEG)
    {
        limitL = 16;
        limitH = 235;
        limitcL = 16;
        limitcH = 239;
    }

    cfg.loLuma = valueLimit(cfg.loLuma,0.0,1.0);
    cfg.mdLuma = valueLimit(cfg.mdLuma,0.0,1.0);
    cfg.hiLuma = valueLimit(cfg.hiLuma,0.0,1.0);
    cfg.loShift = valueLimit(cfg.loShift,0.0,1.0);
    cfg.mdShift = valueLimit(cfg.mdShift,0.0,1.0);
    cfg.hiShift = valueLimit(cfg.hiShift,0.0,1.0);
    cfg.loSaturation = valueLimit(cfg.loSaturation,-1.0,1.0)+1.0;
    cfg.mdSaturation = valueLimit(cfg.mdSaturation,-1.0,1.0)+1.0;
    cfg.hiSaturation = valueLimit(cfg.hiSaturation,-1.0,1.0)+1.0;

    // Luma MAP
    quadraticCurve(mapY, cfg.loLuma, cfg.mdLuma, cfg.hiLuma, 0.0, 1.0, (img->_range == ADM_COL_RANGE_MPEG), 255.0, 220.0, 16.0);

    // Chroma MAP
    double vecU[3],vecV[3];
    vecU[0] = std::sin(cfg.loAngle*(M_PI/180.0))*cfg.loShift;    // WTF?? why U need sine?
    vecV[0] = std::cos(cfg.loAngle*(M_PI/180.0))*cfg.loShift;
    vecU[1] = std::sin(cfg.mdAngle*(M_PI/180.0))*cfg.mdShift;
    vecV[1] = std::cos(cfg.mdAngle*(M_PI/180.0))*cfg.mdShift;
    vecU[2] = std::sin(cfg.hiAngle*(M_PI/180.0))*cfg.hiShift;
    vecV[2] = std::cos(cfg.hiAngle*(M_PI/180.0))*cfg.hiShift;
    quadraticCurve(mapU, vecU[0], vecU[1], vecU[2], -1.0, 1.0, (img->_range == ADM_COL_RANGE_MPEG), 128.0, 112.0, 0.0);
    quadraticCurve(mapV, vecV[0], vecV[1], vecV[2], -1.0, 1.0, (img->_range == ADM_COL_RANGE_MPEG), 128.0, 112.0, 0.0);

    // Saturation MAP
    quadraticCurve(mapS, cfg.loSaturation, cfg.mdSaturation, cfg.hiSaturation, 0.0, 2.0, (img->_range == ADM_COL_RANGE_MPEG), 256.0, 256.0, 0.0);


    // Chroma corrections
    ptr = plane[0];
    ptrU = plane[1];
    ptrV = plane[2];
    int lumas[4];
    int pixel, sumpixel;
    for(int y=0;y<height/2;y++)
    {
        for (int x=0;x<width/2;x++)
        {
            lumas[0] = *(ptr + 2*x);
            lumas[1] = *(ptr + 2*x + 1);
            lumas[2] = *(ptr + stride[0] + 2*x);
            lumas[3] = *(ptr + stride[0] + 2*x + 1);

            pixel = ptrU[x];
            pixel -= 128;
            sumpixel = 0;
            for (int i=0; i<4; i++)
            {
                sumpixel += (((pixel + mapU[lumas[i]]) * mapS[lumas[i]]) >> 8);
            }
            pixel = sumpixel >> 2;
            pixel += 128;
            ptrU[x] = valueLimit(pixel,limitcL,limitcH);


            pixel = ptrV[x];
            pixel -= 128;
            sumpixel = 0;
            for (int i=0; i<4; i++)
            {
                sumpixel += (((pixel + mapV[lumas[i]]) * mapS[lumas[i]]) >> 8);
            }
            pixel = sumpixel >> 2;
            pixel += 128;
            ptrV[x] = valueLimit(pixel,limitcL,limitcH);
        }
        ptr += 2*stride[0];
        ptrU += stride[1];
        ptrV += stride[2];
    }

    // Luma corrections
    ptr = plane[0];
    for(int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            ptr[x] = mapY[ptr[x]];
        }
        ptr += stride[0];
    }

    free(buf);
}
/**
    \fn ColorBalanceRanges_C
*/
void ADMVideoColorBalance::ColorBalanceRanges_C(ADMImage *img)
{
    if (!img) return;
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    uint8_t * ptr, * plane[3];
    int stride[3];

    img->GetReadPlanes(plane);
    img->GetPitches(stride);

    for (int p=1; p<3; p++)
    {
        for(int y=0;y<height/2;y++)
        {
            memset(plane[p],128,width/2);
            plane[p] += stride[p];
        }
    }

    if(img->_range == ADM_COL_RANGE_MPEG)
    {
        ptr = plane[0];
        for(int y=0;y<height;y++)
        {
            for (int x=0;x<width;x++)
            {
                if (ptr[x] < 89)
                    ptr[x] = 16;
                else if (ptr[x] < 163)
                    ptr[x] = 126;
                else
                    ptr[x] = 235;
            }
            ptr += stride[0];
        }
    } else {
        ptr = plane[0];
        for(int y=0;y<height;y++)
        {
            for (int x=0;x<width;x++)
            {
                if (ptr[x] < 85)
                    ptr[x] = 0;
                else if (ptr[x] < 170)
                    ptr[x] = 127;
                else
                    ptr[x] = 255;
            }
            ptr += stride[0];
        }
    }

}
/**
    \fn configure
*/
bool ADMVideoColorBalance::configure()
{
    uint8_t r=0;

    r=  DIA_getColorBalance(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoColorBalance::getConfiguration(void)
{
    static char s[2560];
    snprintf(s,2559,"              [Luma; Angle; Shift; Saturation]\nShadow:   [%.2f; %.0f; %.2f; %.2f]\nMidtone:  [%.2f; %.0f; %.2f; %.2f]\nHighlight: [%.2f; %.0f; %.2f; %.2f]\n", 
                                        _param.loLuma, _param.loAngle, _param.loShift, _param.loSaturation, 
                                        _param.mdLuma, _param.mdAngle, _param.mdShift, _param.mdSaturation, 
                                        _param.hiLuma, _param.hiAngle, _param.hiShift, _param.hiSaturation );
    return s;
}
/**
    \fn ctor
*/
ADMVideoColorBalance::ADMVideoColorBalance(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,colorBalance_param,&_param))
        reset(&_param);
    update();
}
/**
    \fn reset
*/
void ADMVideoColorBalance::reset(colorBalance *cfg)
{
    cfg->loLuma = 0.0;
    cfg->mdLuma = 0.5;
    cfg->hiLuma = 1.0;
    cfg->loAngle = 150.0;
    cfg->mdAngle = 150.0;
    cfg->hiAngle = 150.0;
    cfg->loShift = 0.0;
    cfg->mdShift = 0.0;
    cfg->hiShift = 0.0;
    cfg->loSaturation = 0.0;
    cfg->mdSaturation = 0.0;
    cfg->hiSaturation = 0.0;
}
/**
    \fn valueLimit
*/
float ADMVideoColorBalance::valueLimit(float val, float min, float max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}
/**
    \fn valueLimit
*/
int32_t ADMVideoColorBalance::valueLimit(int32_t val, int32_t min, int32_t max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}
/**
    \fn update
*/
void ADMVideoColorBalance::update(void)
{
    _internalparam.loLuma = valueLimit(_param.loLuma,0.0,1.0);
    _internalparam.mdLuma = valueLimit(_param.mdLuma,0.0,1.0);
    _internalparam.hiLuma = valueLimit(_param.hiLuma,0.0,1.0);
    _internalparam.loAngle = _param.loAngle;
    _internalparam.mdAngle = _param.mdAngle;
    _internalparam.hiAngle = _param.hiAngle;
    _internalparam.loShift = valueLimit(_param.loShift,0.0,1.0);
    _internalparam.mdShift = valueLimit(_param.mdShift,0.0,1.0);
    _internalparam.hiShift = valueLimit(_param.hiShift,0.0,1.0);
    _internalparam.loSaturation = valueLimit(_param.loSaturation,-1.0,1.0);
    _internalparam.mdSaturation = valueLimit(_param.mdSaturation,-1.0,1.0);
    _internalparam.hiSaturation = valueLimit(_param.hiSaturation,-1.0,1.0);
}
/**
    \fn dtor
*/
ADMVideoColorBalance::~ADMVideoColorBalance()
{

}
/**
    \fn getCoupledConf
*/
bool ADMVideoColorBalance::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, colorBalance_param,&_param);
}

void ADMVideoColorBalance::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, colorBalance_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoColorBalance::getNextFrame(uint32_t *fn,ADMImage *image)
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

    ColorBalanceProcess_C(image, _internalparam);

    return 1;
}

