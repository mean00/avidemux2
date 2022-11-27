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

#include "fastFSRCNN.h"

#include "ffx2.h"

int fastFSRCNN::getScaling(int algo)
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



fastFSRCNN::fastFSRCNN(int w, int h, int algo) : NeuronSW(w,h)
{
    paddedImg = new ADMImageDefault(w+4, h+4);
    paddedImgPlane = paddedImg->GetReadPtr(PLANAR_Y);
    paddedImgStride = paddedImg->GetPitch(PLANAR_Y);    
    
    scaling = fastFSRCNN::getScaling(algo);
    
    worker_threads = new pthread_t[threads];
    worker_thread_args = new worker_thread_arg[threads];
    
    
    layerFeature = (float*)ADM_alloc( (w+2)*(h+2)*8* sizeof(float));    // must be aligned
    memset(layerFeature, 0,  (w+2)*(h+2)*8* sizeof(float));
    layerModel1 = (float*)ADM_alloc( (w+2)*(h+2)*8* sizeof(float));    // must be aligned
    memset(layerModel1, 0,  (w+2)*(h+2)*8* sizeof(float));
    layerModel2 = (float*)ADM_alloc( (w+2)*(h+2)*8* sizeof(float));    // must be aligned
    memset(layerModel2, 0,  (w+2)*(h+2)*8* sizeof(float));
    layerModel3 = (float*)ADM_alloc( (w+2)*(h+2)*8* sizeof(float));    // must be aligned
    memset(layerModel3, 0,  (w+2)*(h+2)*8* sizeof(float));
    layerModel4 = (float*)ADM_alloc( (w+2)*(h+2)*8* sizeof(float));    // must be aligned
    memset(layerModel4, 0,  (w+2)*(h+2)*8* sizeof(float));
    layerResidual = (float*)ADM_alloc( (w+2)*(h+2)*8* sizeof(float));    // must be aligned
    memset(layerResidual, 0,  (w+2)*(h+2)*8* sizeof(float));
    
    paramFeatureBias = (float*)ADM_alloc(8 * sizeof(float));    // must be aligned
    paramFeatureWeights = (float*)ADM_alloc(25 * 4 * 2 * sizeof(float));    // must be aligned
    paramModel1Bias = (float*)ADM_alloc(8 * sizeof(float));    // must be aligned
    paramModel1Weights = (float*)ADM_alloc(18 * 16 * 2 * sizeof(float));    // must be aligned
    paramModel1Alpha = (float*)ADM_alloc(8 * sizeof(float));    // must be aligned
    paramModel2Bias = (float*)ADM_alloc(8 * sizeof(float));    // must be aligned
    paramModel2Weights = (float*)ADM_alloc(18 * 16 * 2 * sizeof(float));    // must be aligned
    paramModel2Alpha = (float*)ADM_alloc(8 * sizeof(float));    // must be aligned
    paramModel3Bias = (float*)ADM_alloc(8 * sizeof(float));    // must be aligned
    paramModel3Weights = (float*)ADM_alloc(18 * 16 * 2 * sizeof(float));    // must be aligned
    paramModel3Alpha = (float*)ADM_alloc(8 * sizeof(float));    // must be aligned
    paramModel4Bias = (float*)ADM_alloc(8 * sizeof(float));    // must be aligned
    paramModel4Weights = (float*)ADM_alloc(18 * 16 * 2 * sizeof(float));    // must be aligned
    paramModel4Alpha = (float*)ADM_alloc(8 * sizeof(float));    // must be aligned
    paramResidualBias = (float*)ADM_alloc(8 * sizeof(float));    // must be aligned
    paramResidualWeights = (float*)ADM_alloc(16 * 2 * 2 * sizeof(float));    // must be aligned
    paramResidualAlpha = (float*)ADM_alloc(8 * sizeof(float));    // must be aligned
    paramSubConvolutionBias = (float*)ADM_alloc(4*scaling * sizeof(float));    // must be aligned
    paramSubConvolutionWeights = (float*)ADM_alloc(18 * 16 * scaling * sizeof(float));    // must be aligned
    
    switch(algo) {
        case 0:
            memcpy(paramFeatureBias, ffx2_paramFeatureBias, 8 * sizeof(float));
            memcpy(paramFeatureWeights, ffx2_paramFeatureWeights, 25 * 4 * 2 * sizeof(float));
            memcpy(paramModel1Bias, ffx2_paramModel1Bias, 8 * sizeof(float));
            memcpy(paramModel1Weights, ffx2_paramModel1Weights, 18 * 16 * 2 * sizeof(float));
            memcpy(paramModel1Alpha, ffx2_paramModel1Alpha, 8 * sizeof(float));
            memcpy(paramModel2Bias, ffx2_paramModel2Bias, 8 * sizeof(float));
            memcpy(paramModel2Weights, ffx2_paramModel2Weights, 18 * 16 * 2 * sizeof(float));
            memcpy(paramModel2Alpha, ffx2_paramModel2Alpha, 8 * sizeof(float));
            memcpy(paramModel3Bias, ffx2_paramModel3Bias, 8 * sizeof(float));
            memcpy(paramModel3Weights, ffx2_paramModel3Weights, 18 * 16 * 2 * sizeof(float));
            memcpy(paramModel3Alpha, ffx2_paramModel3Alpha, 8 * sizeof(float));
            memcpy(paramModel4Bias, ffx2_paramModel4Bias, 8 * sizeof(float));
            memcpy(paramModel4Weights, ffx2_paramModel4Weights, 18 * 16 * 2 * sizeof(float));
            memcpy(paramModel4Alpha, ffx2_paramModel4Alpha, 8 * sizeof(float));
            memcpy(paramResidualBias, ffx2_paramResidualBias, 8 * sizeof(float));
            memcpy(paramResidualWeights, ffx2_paramResidualWeights, 16 * 2 * 2 * sizeof(float));
            memcpy(paramResidualAlpha, ffx2_paramResidualAlpha, 8 * sizeof(float));
            memcpy(paramSubConvolutionBias, ffx2_paramSubConvolutionBias, 4*1 * sizeof(float));
            memcpy(paramSubConvolutionWeights, ffx2_paramSubConvolutionWeights, 18 * 16 * 1 * sizeof(float));
            transposeWeights(paramSubConvolutionWeights, 36 * 16 * 1);
            break;
        default:
            ADM_assert(0);
            break;
    }    
    transposeWeights(paramModel1Weights, 18 * 16 * 2);
    transposeWeights(paramModel2Weights, 18 * 16 * 2);
    transposeWeights(paramModel3Weights, 18 * 16 * 2);
    transposeWeights(paramModel4Weights, 18 * 16 * 2);
    transposeWeights(paramResidualWeights, 16 * 2 * 2);
}


fastFSRCNN::~fastFSRCNN()
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


void fastFSRCNN::upscaleY(ADMImage *srcImg, ADMImage *dstImg)
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



void * fastFSRCNN::feature_worker_thread( void *ptr )
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
            m_load2_bias(t, arg->bias);
            int ws = 0;
            for (int p=-2; p<=2; p++)
            {
                for (int q=-2; q<=2; q++)
                {
                    pix = arg->imgPlane[x + 2 + p + (y + 2 + q)*arg->imgStride] / 255.0;
                    m_add2_vecXs(t, arg->weights + ws, pix);
                    ws += 8; 
                }
            }
            m_store2(t, arg->outputLayer + ((y+1)*(w+2) + (x+1))*4*2);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * fastFSRCNN::model_worker_thread( void *ptr )
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
            m_load2_bias(t, arg->bias);
            int ws = 0;
            for (int p=-1; p<=1; p++)
            {
                for (int q=-1; q<=1; q++)
                {
                    m_add2_mxXvec2(t, arg->weights + ws, arg->inputLayer + ((y+1+q)*(w+2) + (x+1+p))*4*2);
                    ws += 64;
                }
            }
            m_alpha2(t, arg->alpha);
            m_store2(t, arg->outputLayer + ((y+1)*(w+2) + (x+1))*4*2);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * fastFSRCNN::residual_worker_thread( void *ptr )
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
            m_load2_bias(t, arg->bias);
            m_add2_mxXvec2(t, arg->weights, arg->inputLayer + ((y+1)*(w+2) + (x+1))*4*2);
            m_add2_vec(t, arg->featureLayer + ((y+1)*(w+2) + (x+1))*4*2);
            m_alpha2(t, arg->alpha);
            m_store2(t, arg->outputLayer + ((y+1)*(w+2) + (x+1))*4*2);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * fastFSRCNN::subconv2_worker_thread( void *ptr )
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

                    m_add1_mxXvec2(t, arg->weights + ws, arg->inputLayer + ((y+1+q)*(w+2) + (x+1+p))*4*2);
                    ws += 32;
                }
            }
            m_integerize1(t, arg->imgPlane + ((y*2)*arg->imgStride + (x*2)), arg->imgStride);
        }
    }
    
    pthread_exit(NULL);
    return NULL;    
}

void * fastFSRCNN::subconv3_worker_thread( void *ptr )
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

void * fastFSRCNN::subconv4_worker_thread( void *ptr )
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

void fastFSRCNN::transposeWeights(float * weights, int weightCount)
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