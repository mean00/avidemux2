/***************************************************************************
                          ImageStab filter ported from frei0r
    Algorithm:
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
#include "ADM_coreVideoFilter.h"
#include "motest.h"

#define MOTION_ESTIMATION_CONTRAST_THRESHOLD	(16)
/**
    \class ADMVideoImageStab
*/
class  ADMVideoImageStab:public ADM_coreVideoFilter
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
        uint8_t *   out;
        int *       bicubicWeights;
    } worker_thread_arg;

    typedef struct {
        uint64_t              prevPts;
        int                   rgbBufStride;
        ADM_byteBuffer *      rgbBufRawIn;
        ADM_byteBuffer *      rgbBufRawOut;
        ADMImageRef *         rgbBufImage;
        ADMColorScalerFull *  convertYuvToRgb;
        ADMColorScalerFull *  convertRgbToYuv;
        int *                 bicubicWeights;
        float                 prevChromaHist[64];
        motest *              motestp;
        double                hist[3];
        double                last[3];
        double                lastSameImage[3];
        bool                  newSceneSameImage;
        float                 sceneDiffSameImage;
        int threads;
        pthread_t  * worker_threads;
        worker_thread_arg * worker_thread_args;
    } imageStab_buffers_t;

  protected:
    void                  update(void);
    imageStab             _param;
    imageStab_buffers_t   _buffers;
    static void           bilinear(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, uint8_t * out);
    static void           bicubic(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, int * weights, uint8_t * out);
    

    static void * worker_thread( void *ptr );
    
  public:
    ADMVideoImageStab(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoImageStab();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface

    static void ImageStabCreateBuffers(int w, int h, imageStab_buffers_t * buffers);
    static void ImageStabDestroyBuffers(imageStab_buffers_t * buffers);
    static void ImageStabProcess_C(ADMImage *img, int w, int h, imageStab param, imageStab_buffers_t * buffers, bool * newScene, float * sceneDiff);
};
