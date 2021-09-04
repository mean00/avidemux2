/***************************************************************************
                          Motion interpolation
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
#include "ADM_image.h"
#include "ADM_default.h"

#define MOTIN_MAX_PYRAMID_LEVELS  (7)
#define MOTIN_SEARCH_RADIUS       (2)

class  motin
{

  protected:
    uint32_t    threads;
    uint32_t    unithreads;
    int         frameW, frameH;
    int         pyramidLevels;
    bool        sceneChanged;
    ADMImage *  frameA;
    ADMImage *  frameB;
    ADMImage ** pyramidA;
    ADMImage ** pyramidB;
    ADMImage ** pyramidWA;
    ADMImage ** pyramidWB;
    ADMColorScalerFull ** upScalersA;
    ADMColorScalerFull ** upScalersB;
    ADMColorScalerFull ** downScalers;
    
    typedef struct {
        int levels;
        ADMColorScalerFull ** scalers;
        ADMImage ** src;
        ADMImage ** dst;
    } scaler_thread_arg;
    
    static void *scaler_thread( void *ptr );
    
    typedef struct {
        int lv;
        uint8_t * plA[3];
        uint8_t * plB[3];
        uint8_t * plW[3];
        int strides[3];
        uint32_t w,h;
        uint32_t ystart, yincr, plane;
    } worker_thread_arg;
    
    pthread_t  * me_threads1;
    pthread_t  * me_threads2;
    worker_thread_arg * worker_thread_args1;
    worker_thread_arg * worker_thread_args2;
    
    static void *me_worker_thread( void *ptr );
    static void *spf_worker_thread( void *ptr );
    static void *tmf_worker_thread( void *ptr );
    
    static int sad(uint8_t * p1, uint8_t * p2, int stride, int x1, int y1, int x2, int y2);
    static void StackBlurLine_C(uint8_t * line, int len, int pixPitch, uint32_t * stack, unsigned int radius);
    
    typedef struct {
        uint8_t * dplanes[3];
        uint8_t * wAplanes[3];
        uint8_t * wBplanes[3];
        uint8_t * pAplanes[3];
        uint8_t * pBplanes[3];
        int dstrides[3];
        int wstrides[3];
        int pstrides[3];
        uint32_t w,h;
        uint32_t ystart, yincr, plane;
        int alpha;
    } uniworker_thread_arg;

    pthread_t  * uniw_threads;
    uniworker_thread_arg * uniworker_thread_args;
    
    static void *interp_worker_thread( void *ptr );

  public:
    motin(int width, int height);
    ~motin();
    
    void createPyramids(ADMImage * imgA, ADMImage * imgB);
    void estimateMotion();
    void interpolate(ADMImage * dst, int alpha);
};
