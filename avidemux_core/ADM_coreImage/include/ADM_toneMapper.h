/***************************************************************************
                       
    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr
    copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_TONEMAPPER_H
#define ADM_TONEMAPPER_H

#include "ADM_coreImage6_export.h"
#include "ADM_rgb.h" // To have colors
#include "ADM_threads.h"

#define DEFAULT_TARGET_LUMINANCE_HDR 100.0

class ADM_COREIMAGE6_EXPORT ADMToneMapperConfig
{
  protected:
    static uint32_t method;
    static float saturation;
    static float boost;
    static bool adaptive;
    static uint32_t gamut;
    float luminance;
    bool changed;
  public:
            ADMToneMapperConfig(bool init=false);
    void    getConfig(uint32_t * toneMappingMethod, float * saturationAdjust, float * boostAdjust, bool * adaptiveRGB, uint32_t * gamutMethod, float * targetLuminance=NULL);
    void    setConfig(uint32_t toneMappingMethod, float saturationAdjust, float boostAdjust, bool adaptiveRGB, uint32_t gamutMethod);
};

class ADMImage;
/**
    \class ADMToneMapper
*/
class ADM_COREIMAGE6_EXPORT ADMToneMapper
{
  protected:
    ADMToneMapperConfig * config;
    void            *contextYUV;
    void            *contextRGB1, *contextRGB2;
    uint32_t        srcWidth,srcHeight;
    uint32_t        dstWidth,dstHeight;
    ADM_pixelFormat fromPixFrmt,toPixFrmt;
    int             sws_flag;
    #define ADM_COLORSPACE_HDR_LUT_WIDTH (12)	// bits
    #define ADM_COLORSPACE_HDR_LUT_SIZE	(1<<ADM_COLORSPACE_HDR_LUT_WIDTH)
    uint8_t         *hdrLumaLUT;
    uint8_t         *hdrChromaBLUT[256];
    uint8_t         *hdrChromaRLUT[256];
    uint8_t         *hdrLumaCrLUT[256];
    uint16_t        *hdrRGBLUT;
    uint16_t         *hdrGammaLUT;
    #define ADM_ADAPTIVE_HDR_LIN_LUT_WIDTH (10)	// bits
    #define ADM_ADAPTIVE_HDR_LIN_LUT_SIZE	(1<<ADM_ADAPTIVE_HDR_LIN_LUT_WIDTH)
    uint16_t        *linearizeLUT;
    double          adaptLLAvg,adaptLLMax;
    int32_t         *adaptHistoPrev,*adaptHistoCurr;
    double          hdrTMsrcLum, hdrTMtrgtLum, hdrTMsat, hdrTMboost;
    unsigned int    hdrTMmethod;
    uint16_t        *hdrYUV;
    uint16_t        *hdrYCbCr[3];
    uint8_t         *sdrYUV[3];
    uint8_t         sdrRGBSat[256];
    uint32_t        threadCount,threadCountYUV;
    pthread_t       *worker_threads;
    
    typedef struct {
        uint32_t        dstWidth,dstHeight;
        uint32_t        ystart, yincr;
        uint8_t         *gbrData[3];
        uint8_t         *dstData[3];
        int             *dstStride;
        bool            p3_primaries;
        uint8_t         *hdrLumaLUT;
        uint8_t         *hdrChromaBLUT[256];
        uint8_t         *hdrChromaRLUT[256];
        uint8_t         *hdrLumaCrLUT[256];
    } fastYUV_worker_thread_arg;
    
    fastYUV_worker_thread_arg *fastYUV_worker_thread_args;

    typedef struct {
        uint32_t        srcWidth,srcHeight;
        uint32_t        ystart, yincr;
        uint16_t        *hdrYCbCr[3];
        uint8_t         *sdrYUV[3];
        uint16_t        *hdrRGBLUT;
        int             *ccmx;
        uint16_t         *hdrGammaLUT;
        unsigned int    gamutMethod;
        uint8_t         *sdrRGBSat;
    } RGB_worker_thread_arg;
    
    RGB_worker_thread_arg *RGB_worker_thread_args;

    typedef struct {
        uint32_t        srcWidth,srcHeight;
        uint32_t        ystart, yincr;
        uint16_t        *hdrY;
        uint16_t        *linearizeLUT;
        uint64_t        partialMax,partialAvg;
    } RGB_peak_measure_thread_arg;
    
    RGB_peak_measure_thread_arg * RGB_peak_measure_thread_args;
    
    typedef struct {
        ADMImage        *sourceImage;
        uint32_t        lstart, lincr;
        unsigned int    method;
        double          gain,npl,boost;
        uint16_t        *hdrRGBLUT;
        uint16_t         *hdrGammaLUT;
    } RGB_LUTgen_thread_arg;
    RGB_LUTgen_thread_arg * RGB_LUTgen_thread_args;
    
    
    static void *   toneMap_fastYUV_worker(void *argptr);
    bool            toneMap_fastYUV(ADMImage *sourceImage, ADMImage *destImage, double targetLuminance, double saturationAdjust, double boostAdjust);
    static void *   toneMap_RGB_worker(void *argptr);
    static void *   toneMap_RGB_peak_measure_worker(void *argptr);
    static void *   toneMap_RGB_LUTgen_worker(void *argptr);
    void            toneMap_RGB_ColorMatrix(int32_t * matrix, ADM_colorPrimaries colorPrim, ADM_colorSpace colorSpace, double * primaries, double * whitePoint);
    bool            toneMap_RGB(ADMImage *sourceImage, ADMImage *destImage, unsigned int method, double targetLuminance, double saturationAdjust, double boostAdjust, bool adaptive, unsigned int gamutMethod);
  public :
                    ADMToneMapper(int sws_flag, int sw, int sh, int dw,int dh,ADM_pixelFormat from,ADM_pixelFormat to);
    bool            toneMap(ADMImage *sourceImage, ADMImage *destImage);
                    ~ADMToneMapper();
};

#endif
//EOF

