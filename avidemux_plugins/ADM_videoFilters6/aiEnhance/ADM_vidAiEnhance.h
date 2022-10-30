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
#pragma once
#include "ADM_default.h"
#include "ADM_threads.h"
#include "ADM_byteBuffer.h"
#include "ADM_image.h"

#ifdef ADM_CPU_X86_64
#include <immintrin.h>
#define USE_SSE2
#endif

class FSRCNN
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
        float *         outputLayer; 
    } worker_thread_arg;
    
    unsigned int            w,h,scaling;
    ADMImage *              paddedImg;
    uint8_t *               paddedImgPlane;
    unsigned int            paddedImgStride;
    unsigned int            threads;
    pthread_t *             worker_threads;
    worker_thread_arg *     worker_thread_args;
    
    float *                 layerFeature;
    float *                 layerModel1;
    float *                 layerModel2;
    float *                 layerModel3;
    float *                 layerModel4;
    float *                 layerResidual;
    
    float *                 paramFeatureBias;
    float *                 paramFeatureWeights;
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
    float *                 paramResidualBias;
    float *                 paramResidualWeights;
    float *                 paramResidualAlpha;
    float *                 paramSubConvolutionBias;
    float *                 paramSubConvolutionWeights;
    
    
           void     transposeWeights(float * weights, int weightCount);
           
    static void *   feature_worker_thread( void *ptr );
    static void *   model_worker_thread( void *ptr );
    static void *   residual_worker_thread( void *ptr );
    static void *   subconv2_worker_thread( void *ptr );
    static void *   subconv3_worker_thread( void *ptr );
    static void *   subconv4_worker_thread( void *ptr );

#ifdef USE_SSE2
    static void     m_load1_bias(__m128 * t, float * bias);
    static void     m_load3_bias(__m128 * t, float * bias);
    static void     m_load4_bias(__m128 * t, float * bias);
    static void     m_add4_vec(__m128 * t, float * vec);
    static void     m_add4_vecXs(__m128 * t, float * vec, float scalar);
    static void     m_store1(__m128 * t, float * layer);
    static void     m_store3(__m128 * t, float * layer);
    static void     m_store4(__m128 * t, float * layer);
    static void     m_add1_mxXvec4(__m128 * t, float * mx, float * vec);
    static void     m_add3_mxXvec4(__m128 * t, float * mx, float * vec);
    static void     m_add4_mxXvec4(__m128 * t, float * mx, float * vec);
    static void     m_alpha4(__m128 * t, float * alpha);
    static void     m_integerize1(__m128 * t, uint8_t * plane, unsigned int stride);
    static void     m_integerize3(__m128 * t, uint8_t * plane, unsigned int stride);
    static void     m_integerize4(__m128 * t, uint8_t * plane, unsigned int stride);
#else
    static void     m_load1_bias(float * t, float * bias);
    static void     m_load3_bias(float * t, float * bias);
    static void     m_load4_bias(float * t, float * bias);
    static void     m_add4_vec(float * t, float * vec);
    static void     m_add4_vecXs(float * t, float * vec, float scalar);
    static void     m_store1(float * t, float * layer);
    static void     m_store3(float * t, float * layer);
    static void     m_store4(float * t, float * layer);
    static void     m_add1_mxXvec4(float * t, float * mx, float * vec);
    static void     m_add3_mxXvec4(float * t, float * mx, float * vec);
    static void     m_add4_mxXvec4(float * t, float * mx, float * vec);
    static void     m_alpha4(float * t, float * alpha);
    static void     m_integerize1(float * t, uint8_t * plane, unsigned int stride);
    static void     m_integerize3(float * t, uint8_t * plane, unsigned int stride);
    static void     m_integerize4(float * t, uint8_t * plane, unsigned int stride);
#endif
public:
    FSRCNN(int w, int h, int algo, int threads);
    ~FSRCNN();
    void            upscaleY(ADMImage *srcImg, ADMImage *dstImg);
    static int      getScaling(int algo);
};


/**
    \class ADMVideoAiEnhance
*/
class  ADMVideoAiEnhance:public ADM_coreVideoFilter
{
  public:
    typedef struct {
        unsigned int          w,h;
        int                   algo;
        FSRCNN *              ai;
        ADMImage *            targetImg;
        ADMImage *            previewImg;
        ADMColorScalerFull *  upScaler;
        ADMColorScalerFull *  previewScaler;
        unsigned int          threads;
    } aiEnhance_buffers_t;

  protected:
    void                  update(void);
    aiEnhance             _param;
    aiEnhance_buffers_t   _buffers;
    ADMImage *            inputImg;
    
  public:
    ADMVideoAiEnhance(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoAiEnhance();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface

    static void AiEnhanceInitializeBuffers(int w, int h, aiEnhance_buffers_t * buffers);
    static void AiEnhanceDestroyBuffers(aiEnhance_buffers_t * buffers);
    static void AiEnhanceProcess_C(ADMImage *srcImg, ADMImage *dstImg, bool previewMode, bool skipProcess, aiEnhance param, aiEnhance_buffers_t * buffers);
    static int  getScaling(aiEnhance param);
};
