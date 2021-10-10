/***************************************************************************
                          QuadTrans filter ported from frei0r
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
#include "ADM_default.h"
#include "ADM_threads.h"
#include "ADM_byteBuffer.h"
#include "ADM_image.h"

/**
    \class ADMVideoQuadTrans
*/
class  ADMVideoQuadTrans:public ADM_coreVideoFilter
{
  public:
    typedef struct {
        int         w,h;
        int         ystart, yincr;
        int         algo;
        int *       integerMap;
        int *       fractionalMap;
        int         stride;
        uint8_t *   in;
        uint8_t *   out;
        int *       bicubicWeights;
        uint8_t     blackLevel;
    } worker_thread_arg;

    typedef struct {
        quadTrans             prevparam;
        ADMImage *            imgCopy;
        int *                 integerMap;
        int *                 fractionalMap;
        int *                 integerMapUV;
        int *                 fractionalMapUV;
        int *                 bicubicWeights;
        int                   threads;
        int                   threadsUV;
        pthread_t *           worker_threads;
        worker_thread_arg *   worker_thread_args;
    } quadTrans_buffers_t;

  protected:
    void                  update(void);
    quadTrans             _param;
    quadTrans_buffers_t   _buffers;
    static void           bilinear(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, uint8_t * out);
    static void           bicubic(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, int * weights, uint8_t * out);
    
    static void * worker_thread( void *ptr );
  public:
    ADMVideoQuadTrans(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoQuadTrans();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface

    static void QuadTransCreateBuffers(int w, int h, quadTrans_buffers_t * buffers);
    static void QuadTransDestroyBuffers(quadTrans_buffers_t * buffers);
    static void QuadTransProcess_C(ADMImage *img, int w, int h, quadTrans param, quadTrans_buffers_t * buffers);
};
