/***************************************************************************
                          Flat360 filter
        Copyright 2023 szlldm
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
    \class ADMVideoFlat360
*/
class  ADMVideoFlat360:public ADM_coreVideoFilter
{
  protected:
      enum CubeFaces {
          FRONT, BACK, RIGHT, LEFT, UP, DOWN
      };
  public:
    typedef struct {
        int         w,h;
        int         ystart, yincr;
        int         algo;
        bool        chroma;
        int *       integerMap;
        int *       fractionalMap;
        int         istride,ostride;
        uint8_t *   in;
        uint8_t *   out;
        int *       bicubicWeights;
        flat360     param;
    } worker_thread_arg;

    typedef struct {
        flat360             prevparam;
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
    } flat360_buffers_t;

  protected:
    void                  update(void);
    flat360             _param;
    flat360_buffers_t   _buffers;
    static void           bilinear(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, uint8_t * out);
    static void           bicubic(int w, int h, int stride, uint8_t * in, int x, int y, int fx, int fy, int * weights, uint8_t * out);
    
    static void         * worker_thread( void *ptr );
    static void         * createMapping_worker_thread( void *ptr );
  public:
    ADMVideoFlat360(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoFlat360();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface

    static void Flat360CreateBuffers(int w, int h, flat360_buffers_t * buffers);
    static void Flat360DestroyBuffers(flat360_buffers_t * buffers);
    static void Flat360Process_C(ADMImage *img, int w, int h, flat360 param, flat360_buffers_t * buffers);
};
