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

class ADM_COREIMAGE6_EXPORT ADMToneMapperConfig
{
  protected:
    static uint32_t method;
    static float saturation;
    static float boost;
  public:
            ADMToneMapperConfig(bool init=false);
    void    getConfig(uint32_t * toneMappingMethod, float * saturationAdjust, float * boostAdjust, float * targetLuminance=NULL);
    void    setConfig(uint32_t toneMappingMethod, float saturationAdjust, float boostAdjust);
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
    uint8_t         *hdrGammaLUT;
    double          hdrTMsrcLum, hdrTMtrgtLum, hdrTMsat, hdrTMboost;
    unsigned int    hdrTMmethod;
    uint16_t        *hdrYUV;
    uint16_t        *hdrRGB[3];
    uint8_t         *sdrRGB[3];
    uint8_t         sdrRGBSat[256];
    uint32_t        threadCount;
    pthread_t       *worker_threads;
    
    typedef struct {
        uint32_t        dstWidth,dstHeight;
        uint32_t        ystart, yincr;
        uint8_t         *gbrData[3];
        uint8_t         *dstData[3];
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
        uint16_t        *hdrRGB[3];
        uint8_t         *sdrRGB[3];
        uint16_t        *hdrRGBLUT;
        int             *ccmx;
        uint8_t         *hdrGammaLUT;
    } RGB_worker_thread_arg;
    
    RGB_worker_thread_arg *RGB_worker_thread_args;

    static void *   toneMap_fastYUV_worker(void *argptr);
    bool            toneMap_fastYUV(ADMImage *sourceImage, ADMImage *destImage, double targetLuminance, double saturationAdjust, double boostAdjust);
    static void *   toneMap_RGB_worker(void *argptr);
    void            toneMap_RGB_ColorMatrix(int32_t * matrix, ADM_colorPrimaries colorPrim, ADM_colorSpace colorSpace, double * primaries, double * whitePoint);
    bool            toneMap_RGB(ADMImage *sourceImage, ADMImage *destImage, unsigned int method, double targetLuminance, double saturationAdjust, double boostAdjust);
  public :
                    ADMToneMapper(int sws_flag, int sw, int sh, int dw,int dh,ADM_pixelFormat from,ADM_pixelFormat to);
    bool            toneMap(ADMImage *sourceImage, ADMImage *destImage);
                    ~ADMToneMapper();
};

#endif
//EOF

