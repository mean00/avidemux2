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
            memcpy(paramFeatureBias, fx2a_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx2a_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx2a_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx2a_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx2a_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx2a_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx2a_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx2a_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx2a_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx2a_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx2a_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx2a_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx2a_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx2a_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx2a_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx2a_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx2a_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx2a_paramSubConvolutionBias, 4*scaling * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx2a_paramSubConvolutionWeights, 36 * 16 * 1 * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * 1);
            break;
        case 1:
            memcpy(paramFeatureBias, fx2d_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx2d_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx2d_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx2d_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx2d_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx2d_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx2d_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx2d_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx2d_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx2d_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx2d_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx2d_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx2d_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx2d_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx2d_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx2d_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx2d_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx2d_paramSubConvolutionBias, 4*scaling * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx2d_paramSubConvolutionWeights, 36 * 16 * 1 * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * 1);
            break;
        case 2:
            memcpy(paramFeatureBias, fx2_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx2_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx2_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx2_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx2_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx2_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx2_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx2_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx2_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx2_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx2_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx2_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx2_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx2_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx2_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx2_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx2_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx2_paramSubConvolutionBias, 4*1 * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx2_paramSubConvolutionWeights, 36 * 16 * 1 * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * 1);
            break;
        case 3:
            memcpy(paramFeatureBias, fx4_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx4_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx4_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx4_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx4_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx4_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx4_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx4_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx4_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx4_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx4_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx4_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx4_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx4_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx4_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx4_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx4_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx42_paramSubConvolutionBias, 4*1 * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx42_paramSubConvolutionWeights, 36 * 16 * 1 * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * 1);
            break;
        case 4:
            memcpy(paramFeatureBias, fx3_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx3_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx3_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx3_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx3_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx3_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx3_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx3_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx3_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx3_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx3_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx3_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx3_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx3_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx3_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx3_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx3_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx3_paramSubConvolutionBias, 4*scaling * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx3_paramSubConvolutionWeights, 36 * 16 * scaling * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * scaling);
            break;
        case 5:
            memcpy(paramFeatureBias, fx4_paramFeatureBias, 16 * sizeof(float));
            memcpy(paramFeatureWeights, fx4_paramFeatureWeights, 25 * 4 * 4 * sizeof(float));
            memcpy(paramModel1Bias, fx4_paramModel1Bias, 16 * sizeof(float));
            memcpy(paramModel1Weights, fx4_paramModel1Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel1Alpha, fx4_paramModel1Alpha, 16 * sizeof(float));
            memcpy(paramModel2Bias, fx4_paramModel2Bias, 16 * sizeof(float));
            memcpy(paramModel2Weights, fx4_paramModel2Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel2Alpha, fx4_paramModel2Alpha, 16 * sizeof(float));
            memcpy(paramModel3Bias, fx4_paramModel3Bias, 16 * sizeof(float));
            memcpy(paramModel3Weights, fx4_paramModel3Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel3Alpha, fx4_paramModel3Alpha, 16 * sizeof(float));
            memcpy(paramModel4Bias, fx4_paramModel4Bias, 16 * sizeof(float));
            memcpy(paramModel4Weights, fx4_paramModel4Weights, 36 * 16 * 4 * sizeof(float));
            memcpy(paramModel4Alpha, fx4_paramModel4Alpha, 16 * sizeof(float));
            memcpy(paramResidualBias, fx4_paramResidualBias, 16 * sizeof(float));
            memcpy(paramResidualWeights, fx4_paramResidualWeights, 16 * 4 * 4 * sizeof(float));
            memcpy(paramResidualAlpha, fx4_paramResidualAlpha, 16 * sizeof(float));
            memcpy(paramSubConvolutionBias, fx4_paramSubConvolutionBias, 4*scaling * sizeof(float));
            memcpy(paramSubConvolutionWeights, fx4_paramSubConvolutionWeights, 36 * 16 * scaling * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * scaling);
            break;
        default:
            ADM_assert(0);
            break;
    }    
    transposeWeights(paramModel1Weights, 36 * 16 * 4);
    transposeWeights(paramModel2Weights, 36 * 16 * 4);
    transposeWeights(paramModel3Weights, 36 * 16 * 4);
    transposeWeights(paramModel4Weights, 36 * 16 * 4);
    transposeWeights(paramResidualWeights, 16 * 4 * 4);
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

    for (int tr=0; tr<threads; tr++)
    {
        switch (scaling)
        {
            case 2:
                pthread_create( &worker_threads[tr], NULL, subconv2_worker_thread, (void*) &worker_thread_args[tr]);
                break;
            case 3:
                pthread_create( &worker_threads[tr], NULL, subconv3_worker_thread, (void*) &worker_thread_args[tr]);
                break;
            case 4:
                pthread_create( &worker_threads[tr], NULL, subconv4_worker_thread, (void*) &worker_thread_args[tr]);
                break;
            default:
                ADM_assert(0);
                break;
        }
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
    
    DECLARE_M_VEC_T(t);
    float pix;
    
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load4_bias(t, arg->bias);
            int ws = 0;
            for (int p=-2; p<=2; p++)
            {
                for (int q=-2; q<=2; q++)
                {
                    pix = arg->imgPlane[x + 2 + p + (y + 2 + q)*arg->imgStride] / 255.0;
                    m_add4_vecXs(t, arg->weights + ws, pix);
                    ws += 16; 
                }
            }
            m_store4(t, arg->outputLayer + ((y+1)*(w+2) + (x+1))*4*4);
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
    
    DECLARE_M_VEC_T(t);
   
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load4_bias(t, arg->bias);
            int ws = 0;
            for (int p=-1; p<=1; p++)
            {
                for (int q=-1; q<=1; q++)
                {
                    m_add4_mxXvec4(t, arg->weights + ws, arg->inputLayer + ((y+1+q)*(w+2) + (x+1+p))*4*4);
                    ws += 256;
                }
            }
            m_alpha4(t, arg->alpha);
            m_store4(t, arg->outputLayer + ((y+1)*(w+2) + (x+1))*4*4);
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
    
    DECLARE_M_VEC_T(t);
   
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load4_bias(t, arg->bias);
            m_add4_mxXvec4(t, arg->weights, arg->inputLayer + ((y+1)*(w+2) + (x+1))*4*4);
            m_add4_vec(t, arg->featureLayer + ((y+1)*(w+2) + (x+1))*4*4);
            m_alpha4(t, arg->alpha);
            m_store4(t, arg->outputLayer + ((y+1)*(w+2) + (x+1))*4*4);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * FSRCNN::subconv2_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    
    DECLARE_M_VEC_T(t);
   
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load1_bias(t, arg->bias);
            int ws = 0;
            for (int p=-1; p<=1; p++)
            {
                for (int q=-1; q<=1; q++)
                {

                    m_add1_mxXvec4(t, arg->weights + ws, arg->inputLayer + ((y+1+q)*(w+2) + (x+1+p))*4*4);
                    ws += 64;
                }
            }
            m_integerize1(t, arg->imgPlane + ((y*2)*arg->imgStride + (x*2)), arg->imgStride);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * FSRCNN::subconv3_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    
    DECLARE_M_VEC_T(t);
    
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load4_bias(t, arg->bias);
            int ws = 0;
            for (int p=-1; p<=1; p++)
            {
                for (int q=-1; q<=1; q++)
                {

                    m_add3_mxXvec4(t, arg->weights + ws, arg->inputLayer + ((y+1+q)*(w+2) + (x+1+p))*4*4);
                    ws += 192;
                }
            }
            m_integerize3(t, arg->imgPlane + ((y*3)*arg->imgStride + (x*3)), arg->imgStride);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * FSRCNN::subconv4_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    
    DECLARE_M_VEC_T(t);
    
    for (int y=ystart; y<h; y+=yincr)
    {
        for (int x=0; x<w; x++)
        {    
            m_load4_bias(t, arg->bias);
            int ws = 0;
            for (int p=-1; p<=1; p++)
            {
                for (int q=-1; q<=1; q++)
                {

                    m_add4_mxXvec4(t, arg->weights + ws, arg->inputLayer + ((y+1+q)*(w+2) + (x+1+p))*4*4);
                    ws += 256;
                }
            }
            m_integerize4(t, arg->imgPlane + ((y*4)*arg->imgStride + (x*4)), arg->imgStride);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void FSRCNN::transposeWeights(float * weights, int weightCount)
{
#ifdef USE_SSE2
    return;
#else
    float tmp[16];
    for (int k=0; k<(weightCount/16); k++)
    {
        for (int i=0; i<4; i++)
        {
            for (int j=0; j<4; j++)
            {
                tmp[j*4+i] = weights[i*4+j];
            }
        }
        for (int i=0; i<16; i++)
        {
            weights[i] = tmp[i];
        }
        weights += 16;
    }
#endif
}