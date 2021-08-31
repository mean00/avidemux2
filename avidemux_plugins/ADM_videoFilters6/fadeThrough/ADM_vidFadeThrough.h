/***************************************************************************
                          FadeThrough filter
    Algorithm:
        Copyright Mario Klingemann
        Copyright Maxim Shemanarev
        Copyright 2010 Marko Cebokli
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
#pragma once

#include <pthread.h>
#include "ADM_default.h"
#include "ADM_byteBuffer.h"
#include "ADM_image.h"
#include "ADM_byteBuffer.h"
#include "ADM_coreVideoFilter.h"

#define MOTION_ESTIMATION_CONTRAST_THRESHOLD	(16)
/**
    \class ADMVideoFadeThrough
*/
class  ADMVideoFadeThrough:public ADM_coreVideoFilter
{
  public:
    typedef struct {
        int         w,h;
        int         ystart, yincr;
        int         algo;
        double *    xs;
        double *    ys;
        int         stride;
        uint8_t *   in;
        uint8_t *   in2;
        uint8_t *   out;
        uint8_t *   out2;
        int *       bicubicWeights;
        uint8_t     blackLevel;
    } qtr_worker_thread_arg;

    typedef struct {
        uint8_t *             lut[3];
        uint32_t              blendRGBcached;
        bool                  blendRangeCached;
        int                   blendYUVcached[3];
        uint32_t              vignetteRGBcached;
        bool                  vignetteRangeCached;
        int                   vignetteYUVcached[3];
        
        int                   rgbBufStride;
        ADM_byteBuffer *      rgbBufRaw;
        ADMImageRef *         rgbBufImage;
        ADMColorScalerFull *  convertYuvToRgb;
        ADMColorScalerFull *  convertRgbToYuv;
        uint32_t *            blurStack;
        
        ADMImage *            imgCopy;
        int *                 bicubicWeights;
        int threads;
        int threadsUV;
        pthread_t  * qtr_worker_threads;
        qtr_worker_thread_arg * qtr_worker_thread_args;
    } fadeThrough_buffers_t;

  protected:
    void                  update(void);
    fadeThrough             _param;
    fadeThrough_buffers_t   _buffers;
    static void           bilinear(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, uint8_t * out);
    static void           bicubic(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, int * weights, uint8_t * out);
    

    static void * qtr_worker_thread( void *ptr );
    
  public:
    ADMVideoFadeThrough(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoFadeThrough();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface

    static void FadeThroughCreateBuffers(int w, int h, fadeThrough_buffers_t * buffers);
    static void FadeThroughDestroyBuffers(fadeThrough_buffers_t * buffers);
    static void FadeThroughProcess_C(ADMImage *img, int w, int h, uint64_t absoluteStartPts, fadeThrough param, fadeThrough_buffers_t * buffers);
    static double TransientPoint(double frac, int transient, double duration);
    static void StackBlurLine_C(uint8_t * line, int len, int pixPitch, uint32_t * stack, unsigned int radius);
    
    static bool IsFadeIn();
    static bool IsFadeOut();
};
