/***************************************************************************
                          AiEnhance filter
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

#include "PL3NET.h"

#include "pl3x2.h"

int PL3NET::getScaling(int algo)
{
    int scaling = -1;
    switch(algo) {
        case 0:
            scaling = 2;
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

PL3NET::PL3NET(int w, int h, int algo) : NeuronSW(w,h)
{
    paddedImg = new ADMImageDefault(w+4, h+4);
    paddedImgPlane = paddedImg->GetReadPtr(PLANAR_Y);
    paddedImgStride = paddedImg->GetPitch(PLANAR_Y);    
    
    scaling = PL3NET::getScaling(algo);
    
    worker_threads = new pthread_t[threads];
    worker_thread_args = new worker_thread_arg[threads];
    
    
    layerFeature = (float*)ADM_alloc( (w+2)*(h+2)*12* sizeof(float));    // must be aligned
    memset(layerFeature, 0,  (w+2)*(h+2)*12* sizeof(float));
    layerPassThroughFeature = (float*)ADM_alloc( (w+2)*(h+2)*12* sizeof(float));    // must be aligned
    memset(layerPassThroughFeature, 0,  (w+2)*(h+2)*12* sizeof(float));
    layerModel1 = (float*)ADM_alloc( (w+2)*(h+2)*12* sizeof(float));    // must be aligned
    memset(layerModel1, 0,  (w+2)*(h+2)*12* sizeof(float));
    layerModel2 = (float*)ADM_alloc( (w+2)*(h+2)*12* sizeof(float));    // must be aligned
    memset(layerModel2, 0,  (w+2)*(h+2)*12* sizeof(float));
    layerModel3 = (float*)ADM_alloc( (w+2)*(h+2)*12* sizeof(float));    // must be aligned
    memset(layerModel3, 0,  (w+2)*(h+2)*12* sizeof(float));
    layerModel4 = (float*)ADM_alloc( (w+2)*(h+2)*12* sizeof(float));    // must be aligned
    memset(layerModel4, 0,  (w+2)*(h+2)*12* sizeof(float));
    layerModel5 = (float*)ADM_alloc( (w+2)*(h+2)*12* sizeof(float));    // must be aligned
    memset(layerModel5, 0,  (w+2)*(h+2)*12* sizeof(float));
    layerModel6 = (float*)ADM_alloc( (w+2)*(h+2)*12* sizeof(float));    // must be aligned
    memset(layerModel6, 0,  (w+2)*(h+2)*12* sizeof(float));
    layerResidual = (float*)ADM_alloc( (w+2)*(h+2)*12* sizeof(float));    // must be aligned
    memset(layerResidual, 0,  (w+2)*(h+2)*12* sizeof(float));
    layerSubConvolution = (uint8_t*)ADM_alloc(w*2*h*2);
    memset(layerSubConvolution, 0, w*2*h*2);
    
    paramFeatureBias = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramFeatureWeights = (float*)ADM_alloc(25 * 12 * sizeof(float));    // must be aligned
    paramPassThroughFeatureBias = (float*)ADM_alloc(4 * sizeof(float));    // must be aligned
    paramPassThroughFeatureWeights = (float*)ADM_alloc(25 * 4 * sizeof(float));    // must be aligned
    paramModel1Bias = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramModel1Weights = (float*)ADM_alloc(18 * 36 * 2 * sizeof(float));    // must be aligned
    paramModel1Alpha = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramModel2Bias = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramModel2Weights = (float*)ADM_alloc(18 * 36 * 2 * sizeof(float));    // must be aligned
    paramModel2Alpha = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramModel3Bias = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramModel3Weights = (float*)ADM_alloc(18 * 36 * 2 * sizeof(float));    // must be aligned
    paramModel3Alpha = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramModel4Bias = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramModel4Weights = (float*)ADM_alloc(18 * 36 * 2 * sizeof(float));    // must be aligned
    paramModel4Alpha = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramModel5Bias = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramModel5Weights = (float*)ADM_alloc(18 * 36 * 2 * sizeof(float));    // must be aligned
    paramModel5Alpha = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramModel6Bias = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramModel6Weights = (float*)ADM_alloc(18 * 36 * 2 * sizeof(float));    // must be aligned
    paramModel6Alpha = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramResidualBias = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramResidualWeights = (float*)ADM_alloc(36 * 2 * 2 * sizeof(float));    // must be aligned
    paramResidualAlpha = (float*)ADM_alloc(12 * sizeof(float));    // must be aligned
    paramSubConvolutionBias = (float*)ADM_alloc(4*scaling * sizeof(float));    // must be aligned
    paramSubConvolutionWeights = (float*)ADM_alloc(18 * 36 * scaling * sizeof(float));    // must be aligned
    
    switch(algo) {
        case 0:
            LOAD_PARAM(pl3x2, paramFeatureBias,      12);
            LOAD_PARAM(pl3x2, paramFeatureWeights,   25*12);
            LOAD_PARAM(pl3x2, paramPassThroughFeatureBias,      4);
            LOAD_PARAM(pl3x2, paramPassThroughFeatureWeights,   25*4);
            LOAD_PARAM(pl3x2, paramModel1Bias,       12);
            LOAD_PARAM(pl3x2, paramModel1Weights,    18*36*2);
            LOAD_PARAM(pl3x2, paramModel1Alpha,      12);
            LOAD_PARAM(pl3x2, paramModel2Bias,       12);
            LOAD_PARAM(pl3x2, paramModel2Weights,    18*36*2);
            LOAD_PARAM(pl3x2, paramModel2Alpha,      12);
            LOAD_PARAM(pl3x2, paramModel3Bias,       12);
            LOAD_PARAM(pl3x2, paramModel3Weights,    18*36*2);
            LOAD_PARAM(pl3x2, paramModel3Alpha,      12);
            LOAD_PARAM(pl3x2, paramModel4Bias,       12);
            LOAD_PARAM(pl3x2, paramModel4Weights,    18*36*2);
            LOAD_PARAM(pl3x2, paramModel4Alpha,      12);
            LOAD_PARAM(pl3x2, paramModel5Bias,       12);
            LOAD_PARAM(pl3x2, paramModel5Weights,    18*36*2);
            LOAD_PARAM(pl3x2, paramModel5Alpha,      12);
            LOAD_PARAM(pl3x2, paramModel6Bias,       12);
            LOAD_PARAM(pl3x2, paramModel6Weights,    18*36*2);
            LOAD_PARAM(pl3x2, paramModel6Alpha,      12);
            LOAD_PARAM(pl3x2, paramResidualBias,     12);
            LOAD_PARAM(pl3x2, paramResidualWeights,  36*2*2);
            LOAD_PARAM(pl3x2, paramResidualAlpha,    12);
            LOAD_PARAM(pl3x2, paramSubConvolutionBias,    4*1);
            LOAD_PARAM(pl3x2, paramSubConvolutionWeights, 18*24*1);
            transposeWeights(12, paramSubConvolutionWeights, 18*24*1);
            break;
        default:
            ADM_assert(0);
            break;
    }    
    transposeWeights(12, paramModel1Weights,    18*36*2);
    transposeWeights(12, paramModel2Weights,    18*36*2);
    transposeWeights(12, paramModel3Weights,    18*36*2);
    transposeWeights(12, paramModel4Weights,    18*36*2);
    transposeWeights(12, paramModel5Weights,    18*36*2);
    transposeWeights(12, paramModel6Weights,    18*36*2);
    transposeWeights(12, paramResidualWeights,  36* 2*2);
}


PL3NET::~PL3NET()
{
    delete paddedImg;
    delete [] worker_threads;
    delete [] worker_thread_args;
    ADM_dezalloc(layerFeature);
    ADM_dezalloc(layerPassThroughFeature);
    ADM_dezalloc(layerModel1);
    ADM_dezalloc(layerModel2);
    ADM_dezalloc(layerModel3);
    ADM_dezalloc(layerModel4);
    ADM_dezalloc(layerModel5);
    ADM_dezalloc(layerModel6);
    ADM_dezalloc(layerResidual);
    ADM_dezalloc(layerSubConvolution);
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
    ADM_dezalloc(paramModel5Bias);
    ADM_dezalloc(paramModel5Weights);
    ADM_dezalloc(paramModel5Alpha);
    ADM_dezalloc(paramModel6Bias);
    ADM_dezalloc(paramModel6Weights);
    ADM_dezalloc(paramModel6Alpha);
    ADM_dezalloc(paramResidualBias);
    ADM_dezalloc(paramResidualWeights);
    ADM_dezalloc(paramResidualAlpha);
    ADM_dezalloc(paramSubConvolutionBias);
    ADM_dezalloc(paramSubConvolutionWeights);
}


void PL3NET::upscaleY(ADMImage *srcImg, ADMImage *dstImg)
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
        worker_thread_args[tr].bias2 = paramPassThroughFeatureBias;
        worker_thread_args[tr].weights2 = paramPassThroughFeatureWeights;
        worker_thread_args[tr].outputLayer = layerFeature;
        worker_thread_args[tr].outputLayer2 = layerPassThroughFeature;
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
    
    // Model5 layer
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
        worker_thread_args[tr].inputLayer = layerModel4;
        worker_thread_args[tr].bias = paramModel5Bias;
        worker_thread_args[tr].weights = paramModel5Weights;
        worker_thread_args[tr].alpha = paramModel5Alpha;
        worker_thread_args[tr].outputLayer = layerModel5;
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
    
    // Model6 layer
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
        worker_thread_args[tr].inputLayer = layerModel5;
        worker_thread_args[tr].bias = paramModel6Bias;
        worker_thread_args[tr].weights = paramModel6Weights;
        worker_thread_args[tr].alpha = paramModel6Alpha;
        worker_thread_args[tr].outputLayer = layerModel6;
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
        worker_thread_args[tr].inputLayer = layerModel6;
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
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w;
        worker_thread_args[tr].h = h;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = layerSubConvolution;
        worker_thread_args[tr].imgStride = w*2;
        worker_thread_args[tr].featureLayer = layerPassThroughFeature;
        worker_thread_args[tr].inputLayer = layerResidual;
        worker_thread_args[tr].bias = paramSubConvolutionBias;
        worker_thread_args[tr].weights = paramSubConvolutionWeights;
        worker_thread_args[tr].alpha = NULL;
        worker_thread_args[tr].outputLayer = NULL;
    }

    ADM_assert(scaling == 2);
    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, subconv_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }
    
    // Postprocessing
    uint8_t * dstImgPlane = dstImg->GetReadPtr(PLANAR_Y);
    unsigned int dstImgStride = dstImg->GetPitch(PLANAR_Y);
    for (int tr=0; tr<threads; tr++)
    {
        worker_thread_args[tr].w = w*2;
        worker_thread_args[tr].h = h*2;
        worker_thread_args[tr].ystart = tr;
        worker_thread_args[tr].yincr = threads;
        worker_thread_args[tr].scaling = scaling;
        worker_thread_args[tr].imgPlane = dstImgPlane;
        worker_thread_args[tr].imgStride = dstImgStride;
        worker_thread_args[tr].featureLayer = NULL;
        worker_thread_args[tr].inputLayer = (float*)layerSubConvolution;
        worker_thread_args[tr].bias = NULL;
        worker_thread_args[tr].weights = NULL;
        worker_thread_args[tr].alpha = NULL;
        worker_thread_args[tr].outputLayer = NULL;
    }

    for (int tr=0; tr<threads; tr++)
    {
        pthread_create( &worker_threads[tr], NULL, postproc_worker_thread, (void*) &worker_thread_args[tr]);
    }
    // work in thread workers...
    for (int tr=0; tr<threads; tr++)
    {
        pthread_join( worker_threads[tr], NULL);
    }    
    

    dstImg->copyInfo(srcImg);
}



void * PL3NET::feature_worker_thread( void *ptr )
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
            plxnet_feature_layer_12(5, arg->scaling, arg->imgPlane + x + 2 + (y + 2)*arg->imgStride, arg->imgStride, arg->outputLayer + ((y+1)*(w+2) + (x+1))*12, arg->bias, arg->weights, arg->outputLayer2 + ((y+1)*(w+2) + (x+1))*4, arg->bias2, arg->weights2);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * PL3NET::model_worker_thread( void *ptr )
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
            fsrcnn_model_layer_12(3, arg->inputLayer + (x + 1 + (y + 1)*(w+2))*12, (w+2)*12, arg->outputLayer + (x + 1 + (y + 1)*(w+2))*12, arg->bias, arg->weights, arg->alpha);
        }
    }
    pthread_exit(NULL);
    return NULL;    
}

void * PL3NET::residual_worker_thread( void *ptr )
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
            fsrcnn_residual_layer_12(1, arg->inputLayer + (x + 1 + (y + 1)*(w+2))*12, (w+2)*12, arg->featureLayer + ((y+1)*(w+2) + (x+1))*12, arg->outputLayer + (x + 1 + (y + 1)*(w+2))*12, arg->bias, arg->weights, arg->alpha);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * PL3NET::subconv_worker_thread( void *ptr )
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
            plxnet_subconvolutional_layer_12(3, arg->scaling, arg->inputLayer + (x + 1 + (y + 1)*(w+2))*12, (w+2)*12, arg->featureLayer + (x + 1 + (y + 1)*(w+2))*arg->scaling*arg->scaling, arg->imgPlane + (y*arg->imgStride + x)*arg->scaling, arg->imgStride, arg->bias, arg->weights);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

#define FCOEF00    848
#define FCOEF10    115
#define FCOEF11    -61
#define FCOEF20    -21
#define FCOEF21    3
#define FCOEF22    5

void * PL3NET::postproc_worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    int w = arg->w;
    int h = arg->h;
    int ystart = arg->ystart;
    int yincr = arg->yincr;
    uint8_t * src = (uint8_t*)arg->inputLayer;
    
    for (int y=ystart; y<h; y+=yincr)
    {
        if ((y < 2) || (y >= h-2))
        {
            memcpy(arg->imgPlane + y*arg->imgStride, src + y*w, w);
            continue;
        }
        for (int x=0; x<2; x++)
        {
            arg->imgPlane[y*arg->imgStride + x] = src[y*w + x];
        }
        uint8_t * ptr = (src + (y-2)*w);
        for (int x=2; x<(w-2); x++)
        {   
            uint8_t * ptrl = ptr+(x-2);
            int sum = 0;
            sum += (ptrl[0]*FCOEF22 + ptrl[1]*FCOEF21 + ptrl[2]*FCOEF20 + ptrl[3]*FCOEF21 + ptrl[4]*FCOEF22);
            ptrl += w;
            sum += (ptrl[0]*FCOEF21 + ptrl[1]*FCOEF11 + ptrl[2]*FCOEF10 + ptrl[3]*FCOEF11 + ptrl[4]*FCOEF21);
            ptrl += w;
            sum += (ptrl[0]*FCOEF20 + ptrl[1]*FCOEF10 + ptrl[2]*FCOEF00 + ptrl[3]*FCOEF10 + ptrl[4]*FCOEF20);
            ptrl += w;
            sum += (ptrl[0]*FCOEF21 + ptrl[1]*FCOEF11 + ptrl[2]*FCOEF10 + ptrl[3]*FCOEF11 + ptrl[4]*FCOEF21);
            ptrl += w;
            sum += (ptrl[0]*FCOEF22 + ptrl[1]*FCOEF21 + ptrl[2]*FCOEF20 + ptrl[3]*FCOEF21 + ptrl[4]*FCOEF22);
            sum /= 1024;
            if (sum < 0) sum = 0;
            if (sum > 255) sum = 255;
            arg->imgPlane[y*arg->imgStride + x] = sum;
        }
        for (int x=(w-2); x<w; x++)
        {
            arg->imgPlane[y*arg->imgStride + x] = src[y*w + x];
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}