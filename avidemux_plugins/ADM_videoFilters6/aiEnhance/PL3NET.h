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

#pragma once
#include "NeuronSW.h"

class PL3NET : public NeuronSW
{
private:
    typedef struct {
        int             w,h;
        int             ystart, yincr;
        int             scaling;
        uint8_t *       imgPlane;
        unsigned int    imgStride;
        float *         featureLayer;
        float *         inputLayer;
        float *         bias;
        float *         weights;
        float *         alpha;
        float *         bias2;
        float *         weights2;
        float *         outputLayer; 
        float *         outputLayer2; 
    } worker_thread_arg;
    
    unsigned int            scaling;
    ADMImage *              paddedImg;
    uint8_t *               paddedImgPlane;
    unsigned int            paddedImgStride;
    pthread_t *             worker_threads;
    worker_thread_arg *     worker_thread_args;
    
    float *                 layerFeature;
    float *                 layerPassThroughFeature;
    float *                 layerModel1;
    float *                 layerModel2;
    float *                 layerModel3;
    float *                 layerModel4;
    float *                 layerModel5;
    float *                 layerModel6;
    float *                 layerResidual;
    uint8_t *               layerSubConvolution;
    
    float *                 paramFeatureBias;
    float *                 paramFeatureWeights;
    float *                 paramPassThroughFeatureBias;
    float *                 paramPassThroughFeatureWeights;
    float *                 paramModel1Bias;
    float *                 paramModel1Weights;
    float *                 paramModel1Alpha;
    float *                 paramModel2Bias;
    float *                 paramModel2Weights;
    float *                 paramModel2Alpha;
    float *                 paramModel3Bias;
    float *                 paramModel3Weights;
    float *                 paramModel3Alpha;
    float *                 paramModel4Bias;
    float *                 paramModel4Weights;
    float *                 paramModel4Alpha;
    float *                 paramModel5Bias;
    float *                 paramModel5Weights;
    float *                 paramModel5Alpha;
    float *                 paramModel6Bias;
    float *                 paramModel6Weights;
    float *                 paramModel6Alpha;
    float *                 paramResidualBias;
    float *                 paramResidualWeights;
    float *                 paramResidualAlpha;
    float *                 paramSubConvolutionBias;
    float *                 paramSubConvolutionWeights;

           
    static void *   feature_worker_thread( void *ptr );
    static void *   model_worker_thread( void *ptr );
    static void *   residual_worker_thread( void *ptr );
    static void *   subconv_worker_thread( void *ptr );
    static void *   postproc_worker_thread( void *ptr );

public:
    PL3NET(int w, int h, int algo);
    ~PL3NET();
    void            upscaleY(ADMImage *srcImg, ADMImage *dstImg);
    static int      getScaling(int algo);
};
