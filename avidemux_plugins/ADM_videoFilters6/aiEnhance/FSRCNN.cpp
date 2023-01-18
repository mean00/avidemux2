/***************************************************************************
                          AiEnhance filter
        Copyright 2022 szlldm
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "FSRCNN.h"

#include "fx2a.h"
#include "fx2d.h"
#include "fx2.h"
#include "fx3.h"
#include "fx4.h"

int FSRCNN::getScaling(int algo)
{
    int scaling = -1;
    switch(algo) {
        case 0:
        case 1:
        case 2:
        case 3:
            scaling = 2;
            break;
        case 4:
            scaling = 3;
            break;
        case 5:
            scaling = 4;
            break;
        default:
            ADM_assert(0);
            break;
    }
    return scaling;
}

#define dLOAD_PARAM(PREFIXEDNAME, PARNAME, AMOUNT) \
        for (int i=0; i<(AMOUNT); i++) \
        { \
            PARNAME[i] = PREFIXEDNAME[i] / 8192.0; \
        }
#define LOAD_PARAM(PREFIX, PARNAME, AMOUNT)   dLOAD_PARAM(PREFIX ## _ ## PARNAME, PARNAME, AMOUNT)  

FSRCNN::FSRCNN(int w, int h, int algo) : NeuronSW(w,h)
{
    paddedImg = new ADMImageDefault(w+4, h+4);
    paddedImgPlane = paddedImg->GetReadPtr(PLANAR_Y);
    paddedImgStride = paddedImg->GetPitch(PLANAR_Y);    
    
    scaling = FSRCNN::getScaling(algo);
    
    worker_threads = new pthread_t[threads];
    worker_thread_args = new worker_thread_arg[threads];
    
    
    layerFeature = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerFeature, 0,  (w+2)*(h+2)*16* sizeof(float));
    layerModel1 = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerModel1, 0,  (w+2)*(h+2)*16* sizeof(float));
    layerModel2 = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerModel2, 0,  (w+2)*(h+2)*16* sizeof(float));
    layerModel3 = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerModel3, 0,  (w+2)*(h+2)*16* sizeof(float));
    layerModel4 = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerModel4, 0,  (w+2)*(h+2)*16* sizeof(float));
    layerResidual = (float*)ADM_alloc( (w+2)*(h+2)*16* sizeof(float));    // must be aligned
    memset(layerResidual, 0,  (w+2)*(h+2)*16* sizeof(float));
    
    paramFeatureBias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramFeatureWeights = (float*)ADM_alloc(25 * 4 * 4 * sizeof(float));    // must be aligned
    paramModel1Bias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel1Weights = (float*)ADM_alloc(36 * 16 * 4 * sizeof(float));    // must be aligned
    paramModel1Alpha = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel2Bias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel2Weights = (float*)ADM_alloc(36 * 16 * 4 * sizeof(float));    // must be aligned
    paramModel2Alpha = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel3Bias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel3Weights = (float*)ADM_alloc(36 * 16 * 4 * sizeof(float));    // must be aligned
    paramModel3Alpha = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel4Bias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramModel4Weights = (float*)ADM_alloc(36 * 16 * 4 * sizeof(float));    // must be aligned
    paramModel4Alpha = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramResidualBias = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramResidualWeights = (float*)ADM_alloc(16 * 4 * 4 * sizeof(float));    // must be aligned
    paramResidualAlpha = (float*)ADM_alloc(16 * sizeof(float));    // must be aligned
    paramSubConvolutionBias = (float*)ADM_alloc(4*scaling * sizeof(float));    // must be aligned
    paramSubConvolutionWeights = (float*)ADM_alloc(36 * 16 * scaling * sizeof(float));    // must be aligned
    
    switch(algo) {
        case 0:
            LOAD_PARAM(fx2a, paramFeatureBias,      16);
            LOAD_PARAM(fx2a, paramFeatureWeights,   25*4*4);
            LOAD_PARAM(fx2a, paramModel1Bias,       16);
            LOAD_PARAM(fx2a, paramModel1Weights,    36*16*4);
            LOAD_PARAM(fx2a, paramModel1Alpha,      16);
            LOAD_PARAM(fx2a, paramModel2Bias,       16);
            LOAD_PARAM(fx2a, paramModel2Weights,    36*16*4);
            LOAD_PARAM(fx2a, paramModel2Alpha,      16);
            LOAD_PARAM(fx2a, paramModel3Bias,       16);
            LOAD_PARAM(fx2a, paramModel3Weights,    36*16*4);
            LOAD_PARAM(fx2a, paramModel3Alpha,      16);
            LOAD_PARAM(fx2a, paramModel4Bias,       16);
            LOAD_PARAM(fx2a, paramModel4Weights,    36*16*4);
            LOAD_PARAM(fx2a, paramModel4Alpha,      16);
            LOAD_PARAM(fx2a, paramResidualBias,     16);
            LOAD_PARAM(fx2a, paramResidualWeights,  16*4*4);
            LOAD_PARAM(fx2a, paramResidualAlpha,    16);
            LOAD_PARAM(fx2a, paramSubConvolutionBias,    4*1);
            LOAD_PARAM(fx2a, paramSubConvolutionWeights, 36*16*1);
            transposeWeights(16, paramSubConvolutionWeights, 36*16*1);            
            break;
        case 1:
            LOAD_PARAM(fx2d, paramFeatureBias,      16);
            LOAD_PARAM(fx2d, paramFeatureWeights,   25*4*4);
            LOAD_PARAM(fx2d, paramModel1Bias,       16);
            LOAD_PARAM(fx2d, paramModel1Weights,    36*16*4);
            LOAD_PARAM(fx2d, paramModel1Alpha,      16);
            LOAD_PARAM(fx2d, paramModel2Bias,       16);
            LOAD_PARAM(fx2d, paramModel2Weights,    36*16*4);
            LOAD_PARAM(fx2d, paramModel2Alpha,      16);
            LOAD_PARAM(fx2d, paramModel3Bias,       16);
            LOAD_PARAM(fx2d, paramModel3Weights,    36*16*4);
            LOAD_PARAM(fx2d, paramModel3Alpha,      16);
            LOAD_PARAM(fx2d, paramModel4Bias,       16);
            LOAD_PARAM(fx2d, paramModel4Weights,    36*16*4);
            LOAD_PARAM(fx2d, paramModel4Alpha,      16);
            LOAD_PARAM(fx2d, paramResidualBias,     16);
            LOAD_PARAM(fx2d, paramResidualWeights,  16*4*4);
            LOAD_PARAM(fx2d, paramResidualAlpha,    16);
            LOAD_PARAM(fx2d, paramSubConvolutionBias,    4*1);
            LOAD_PARAM(fx2d, paramSubConvolutionWeights, 36*16*1);
            transposeWeights(16, paramSubConvolutionWeights, 36*16*1); 
            break;
        case 2:
            LOAD_PARAM(fx2, paramFeatureBias,      16);
            LOAD_PARAM(fx2, paramFeatureWeights,   25*4*4);
            LOAD_PARAM(fx2, paramModel1Bias,       16);
            LOAD_PARAM(fx2, paramModel1Weights,    36*16*4);
            LOAD_PARAM(fx2, paramModel1Alpha,      16);
            LOAD_PARAM(fx2, paramModel2Bias,       16);
            LOAD_PARAM(fx2, paramModel2Weights,    36*16*4);
            LOAD_PARAM(fx2, paramModel2Alpha,      16);
            LOAD_PARAM(fx2, paramModel3Bias,       16);
            LOAD_PARAM(fx2, paramModel3Weights,    36*16*4);
            LOAD_PARAM(fx2, paramModel3Alpha,      16);
            LOAD_PARAM(fx2, paramModel4Bias,       16);
            LOAD_PARAM(fx2, paramModel4Weights,    36*16*4);
            LOAD_PARAM(fx2, paramModel4Alpha,      16);
            LOAD_PARAM(fx2, paramResidualBias,     16);
            LOAD_PARAM(fx2, paramResidualWeights,  16*4*4);
            LOAD_PARAM(fx2, paramResidualAlpha,    16);
            LOAD_PARAM(fx2, paramSubConvolutionBias,    4*1);
            LOAD_PARAM(fx2, paramSubConvolutionWeights, 36*16*1);
            transposeWeights(16, paramSubConvolutionWeights, 36*16*1); 
            break;
        case 3:
            LOAD_PARAM(fx4, paramFeatureBias,      16);
            LOAD_PARAM(fx4, paramFeatureWeights,   25*4*4);
            LOAD_PARAM(fx4, paramModel1Bias,       16);
            LOAD_PARAM(fx4, paramModel1Weights,    36*16*4);
            LOAD_PARAM(fx4, paramModel1Alpha,      16);
            LOAD_PARAM(fx4, paramModel2Bias,       16);
            LOAD_PARAM(fx4, paramModel2Weights,    36*16*4);
            LOAD_PARAM(fx4, paramModel2Alpha,      16);
            LOAD_PARAM(fx4, paramModel3Bias,       16);
            LOAD_PARAM(fx4, paramModel3Weights,    36*16*4);
            LOAD_PARAM(fx4, paramModel3Alpha,      16);
            LOAD_PARAM(fx4, paramModel4Bias,       16);
            LOAD_PARAM(fx4, paramModel4Weights,    36*16*4);
            LOAD_PARAM(fx4, paramModel4Alpha,      16);
            LOAD_PARAM(fx4, paramResidualBias,     16);
            LOAD_PARAM(fx4, paramResidualWeights,  16*4*4);
            LOAD_PARAM(fx4, paramResidualAlpha,    16);
            LOAD_PARAM(fx42, paramSubConvolutionBias,    4*1);
            LOAD_PARAM(fx42, paramSubConvolutionWeights, 36*16*1);
            transposeWeights(16, paramSubConvolutionWeights, 36*16*1);             
            break;
        case 4:
            LOAD_PARAM(fx3, paramFeatureBias,      16);
            LOAD_PARAM(fx3, paramFeatureWeights,   25*4*4);
            LOAD_PARAM(fx3, paramModel1Bias,       16);
            LOAD_PARAM(fx3, paramModel1Weights,    36*16*4);
            LOAD_PARAM(fx3, paramModel1Alpha,      16);
            LOAD_PARAM(fx3, paramModel2Bias,       16);
            LOAD_PARAM(fx3, paramModel2Weights,    36*16*4);
            LOAD_PARAM(fx3, paramModel2Alpha,      16);
            LOAD_PARAM(fx3, paramModel3Bias,       16);
            LOAD_PARAM(fx3, paramModel3Weights,    36*16*4);
            LOAD_PARAM(fx3, paramModel3Alpha,      16);
            LOAD_PARAM(fx3, paramModel4Bias,       16);
            LOAD_PARAM(fx3, paramModel4Weights,    36*16*4);
            LOAD_PARAM(fx3, paramModel4Alpha,      16);
            LOAD_PARAM(fx3, paramResidualBias,     16);
            LOAD_PARAM(fx3, paramResidualWeights,  16*4*4);
            LOAD_PARAM(fx3, paramResidualAlpha,    16);
            LOAD_PARAM(fx3, paramSubConvolutionBias,    4*3);
            LOAD_PARAM(fx3, paramSubConvolutionWeights, 36*16*3);
            transposeWeights(16, paramSubConvolutionWeights, 36*16*3);             
            break;
        case 5:
            LOAD_PARAM(fx4, paramFeatureBias,      16);
            LOAD_PARAM(fx4, paramFeatureWeights,   25*4*4);
            LOAD_PARAM(fx4, paramModel1Bias,       16);
            LOAD_PARAM(fx4, paramModel1Weights,    36*16*4);
            LOAD_PARAM(fx4, paramModel1Alpha,      16);
            LOAD_PARAM(fx4, paramModel2Bias,       16);
            LOAD_PARAM(fx4, paramModel2Weights,    36*16*4);
            LOAD_PARAM(fx4, paramModel2Alpha,      16);
            LOAD_PARAM(fx4, paramModel3Bias,       16);
            LOAD_PARAM(fx4, paramModel3Weights,    36*16*4);
            LOAD_PARAM(fx4, paramModel3Alpha,      16);
            LOAD_PARAM(fx4, paramModel4Bias,       16);
            LOAD_PARAM(fx4, paramModel4Weights,    36*16*4);
            LOAD_PARAM(fx4, paramModel4Alpha,      16);
            LOAD_PARAM(fx4, paramResidualBias,     16);
            LOAD_PARAM(fx4, paramResidualWeights,  16*4*4);
            LOAD_PARAM(fx4, paramResidualAlpha,    16);
            LOAD_PARAM(fx4, paramSubConvolutionBias,    4*4);
            LOAD_PARAM(fx4, paramSubConvolutionWeights, 36*16*4);
            transposeWeights(16, paramSubConvolutionWeights, 36*16*4);             
            break;
        default:
            ADM_assert(0);
            break;
    }    
    transposeWeights(16, paramModel1Weights, 36 * 16 * 4);
    shuffleWeights(16, paramModel1Weights,   36 * 16 * 4);
    transposeWeights(16, paramModel2Weights, 36 * 16 * 4);
    shuffleWeights(16, paramModel2Weights,   36 * 16 * 4);
    transposeWeights(16, paramModel3Weights, 36 * 16 * 4);
    shuffleWeights(16, paramModel3Weights,   36 * 16 * 4);
    transposeWeights(16, paramModel4Weights, 36 * 16 * 4);
    shuffleWeights(16, paramModel4Weights,   36 * 16 * 4);
    transposeWeights(16, paramResidualWeights, 16 * 4 * 4);
    shuffleWeights(16, paramResidualWeights,   16 * 4 * 4);
}


FSRCNN::~FSRCNN()
{
    delete paddedImg;
    delete [] worker_threads;
    delete [] worker_thread_args;
    ADM_dezalloc(layerFeature);
    ADM_dezalloc(layerModel1);
    ADM_dezalloc(layerModel2);
    ADM_dezalloc(layerModel3);
    ADM_dezalloc(layerModel4);
    ADM_dezalloc(layerResidual);
    ADM_dezalloc(paramFeatureBias);
    ADM_dezalloc(paramFeatureWeights);
    ADM_dezalloc(paramModel1Bias);
    ADM_dezalloc(paramModel1Weights);
    ADM_dezalloc(paramModel1Alpha);
    ADM_dezalloc(paramModel2Bias);
    ADM_dezalloc(paramModel2Weights);
    ADM_dezalloc(paramModel2Alpha);
    ADM_dezalloc(paramModel3Bias);
    ADM_dezalloc(paramModel3Weights);
    ADM_dezalloc(paramModel3Alpha);
    ADM_dezalloc(paramModel4Bias);
    ADM_dezalloc(paramModel4Weights);
    ADM_dezalloc(paramModel4Alpha);
    ADM_dezalloc(paramResidualBias);
    ADM_dezalloc(paramResidualWeights);
    ADM_dezalloc(paramResidualAlpha);
    ADM_dezalloc(paramSubConvolutionBias);
    ADM_dezalloc(paramSubConvolutionWeights);
}


void FSRCNN::upscaleY(ADMImage *srcImg, ADMImage *dstImg)
{
    ADM_assert(srcImg->_width == w);
    ADM_assert(srcImg->_height == h);
    ADM_assert(dstImg->_width == w*scaling);
    ADM_assert(dstImg->_height == h*scaling);

    // create padded Y plane
    uint8_t * srcImgPlane = srcImg->GetReadPtr(PLANAR_Y);
    unsigned int srcImgStride = srcImg->GetPitch(PLANAR_Y);
    uint8_t * pp = paddedImgPlane;
    pp += 2*paddedImgStride;
    for (int y=0; y<h; y++)
    {
        memcpy(pp+2, srcImgPlane, w);
        pp += paddedImgStride;
        srcImgPlane += srcImgStride;
    }
    pp = paddedImgPlane;
    pp += 2*paddedImgStride;    
    for (int y=0; y<h; y++)
    {
        pp[0] = pp[1] = pp[2];
        pp[w+3] = pp[w+2] = pp[w+1];
        pp += paddedImgStride;    
    }
    memcpy(paddedImgPlane, paddedImgPlane + 2*paddedImgStride, w+4);
    memcpy(paddedImgPlane + paddedImgStride, paddedImgPlane + 2*paddedImgStride, w+4);
    memcpy(paddedImgPlane + (h+2)*paddedImgStride, paddedImgPlane + (h+1)*paddedImgStride, w+4);
    memcpy(paddedImgPlane + (h+3)*paddedImgStride, paddedImgPlane + (h+1)*paddedImgStride, w+4);
    // got padded Y plane
    
    // Feature layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = paddedImgPlane;
        worker_thread_args[tr].imgStride = paddedImgStride;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = NULL;
        worker_thread_args[tr].bias = paramFeatureBias;
        worker_thread_args[tr].weights = paramFeatureWeights;
        worker_thread_args[tr].alpha = NULL;
        worker_thread_args[tr].outputLayer = layerFeature;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, feature_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }
    
    // Model1 layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = NULL;
        worker_thread_args[tr].imgStride = 0;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = layerFeature;
        worker_thread_args[tr].bias = paramModel1Bias;
        worker_thread_args[tr].weights = paramModel1Weights;
        worker_thread_args[tr].alpha = paramModel1Alpha;
        worker_thread_args[tr].outputLayer = layerModel1;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, model_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }

    // Model2 layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = NULL;
        worker_thread_args[tr].imgStride = 0;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = layerModel1;
        worker_thread_args[tr].bias = paramModel2Bias;
        worker_thread_args[tr].weights = paramModel2Weights;
        worker_thread_args[tr].alpha = paramModel2Alpha;
        worker_thread_args[tr].outputLayer = layerModel2;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, model_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }
    
    // Model3 layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = NULL;
        worker_thread_args[tr].imgStride = 0;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = layerModel2;
        worker_thread_args[tr].bias = paramModel3Bias;
        worker_thread_args[tr].weights = paramModel3Weights;
        worker_thread_args[tr].alpha = paramModel3Alpha;
        worker_thread_args[tr].outputLayer = layerModel3;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, model_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }
    
    // Model4 layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = NULL;
        worker_thread_args[tr].imgStride = 0;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = layerModel3;
        worker_thread_args[tr].bias = paramModel4Bias;
        worker_thread_args[tr].weights = paramModel4Weights;
        worker_thread_args[tr].alpha = paramModel4Alpha;
        worker_thread_args[tr].outputLayer = layerModel4;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, model_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }
    
    // Residual layer
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = NULL;
        worker_thread_args[tr].imgStride = 0;
        worker_thread_args[tr].featureLayer = layerFeature;
        worker_thread_args[tr].inputLayer = layerModel4;
        worker_thread_args[tr].bias = paramResidualBias;
        worker_thread_args[tr].weights = paramResidualWeights;
        worker_thread_args[tr].alpha = paramResidualAlpha;
        worker_thread_args[tr].outputLayer = layerResidual;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, residual_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }

    // Subconvolutional layer
    uint8_t * dstImgPlane = dstImg->GetReadPtr(PLANAR_Y);
    unsigned int dstImgStride = dstImg->GetPitch(PLANAR_Y);
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = dstImgPlane;
        worker_thread_args[tr].imgStride = dstImgStride;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = layerResidual;
        worker_thread_args[tr].bias = paramSubConvolutionBias;
        worker_thread_args[tr].weights = paramSubConvolutionWeights;
        worker_thread_args[tr].alpha = NULL;
        worker_thread_args[tr].outputLayer = NULL;
    }

    ADM_assert((scaling >= 2) && (scaling <= 4));
    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, subconv_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }

    dstImg->copyInfo(srcImg);
}



void * FSRCNN::feature_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {
            fsrcnn_feature_layer_16(5, arg->imgPlane + x + 2 + (y + 2)*arg->imgStride, arg->imgStride, arg->outputLayer + ((y+1)*(w+2) + (x+1))*16, arg->bias, arg->weights);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * FSRCNN::model_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
   
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {
            fsrcnn_model_layer_16(3, arg->inputLayer + (x + 1 + (y + 1)*(w+2))*16, (w+2)*16, arg->outputLayer + (x + 1 + (y + 1)*(w+2))*16, arg->bias, arg->weights, arg->alpha);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * FSRCNN::residual_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
   
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {   
            fsrcnn_residual_layer_16(1, arg->inputLayer + (x + 1 + (y + 1)*(w+2))*16, (w+2)*16, arg->featureLayer + ((y+1)*(w+2) + (x+1))*16, arg->outputLayer + (x + 1 + (y + 1)*(w+2))*16, arg->bias, arg->weights, arg->alpha);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * FSRCNN::subconv_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
   
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {   
            fsrcnn_subconvolutional_layer_16(3, arg->scaling, arg->inputLayer + (x + 1 + (y + 1)*(w+2))*16, (w+2)*16, arg->imgPlane + (y*arg->imgStride + x)*arg->scaling, arg->imgStride, arg->bias, arg->weights);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}
