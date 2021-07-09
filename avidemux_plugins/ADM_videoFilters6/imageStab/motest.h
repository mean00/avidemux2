/***************************************************************************
                          Motion estimation
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

#define MOTEST_MAX_PYRAMID_LEVELS  (7)
#define MOTEST_SEARCH_RADIUS       (2)

class  motest
{

  protected:
    uint32_t    threads;
    int         frameW, frameH;
    int         validPrevFrame;
    int         pyramidLevels;
    int         contrastThreshold;
    ADMImage *  frameA;
    ADMImage *  frameB;
    ADMImage ** pyramidA;
    ADMImage ** pyramidB;
    ADMImage ** pyramidWA;
    ADMColorScalerFull ** upScalers;
    ADMColorScalerFull ** downScalers;
    int *       motionMap[2];
    int *       contrastMap;
    double *    angleMap;
    
    typedef struct {
        int lv;
        uint8_t * plA[3];
        uint8_t * plB[3];
        uint8_t * plW[3];
        int strides[3];
        uint32_t w,h;
        uint32_t ystart, yincr;
        unsigned int speedup;
        int * motionMap[2];
        int * contrastMap;
    } worker_thread_arg;
    
    pthread_t  * me_threads1;
    pthread_t  * me_threads2;
    worker_thread_arg * worker_thread_args1;
    worker_thread_arg * worker_thread_args2;
    
    static void *me_worker_thread( void *ptr );
    static void *spf_worker_thread( void *ptr );
    
    static int sad(uint8_t * p1, uint8_t * p2, int stride, int x1, int y1, int x2, int y2);

  public:
    motest(int width, int height, int minContrast);
    ~motest();
    
    void addNextImage(ADMImage * img);
    void estimateMotion(unsigned int speed);
    void getMotionParameters(double * global, double * rotation);
};
