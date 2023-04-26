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
#include "ADM_image.h"
#include "ADM_cpuSIMD.h"

class NeuronSW
{
private:
    static void     fsrcnn_feature_layer_C(int features, int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights);
    static void     fsrcnn_model_layer_C(int features, int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_residual_layer_C(int features, int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_subconvolutional_layer_C(int features, int kernel_size, int scale, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights);
    
    static void     plxnet_feature_layer_C(int features, int kernel_size, int scale, uint8_t * input, int input_stride, float * output, float * bias, float * weights, float * output2, float * bias2, float * weights2);
    static void     plxnet_subconvolutional_layer_C(int features, int kernel_size, int scale, float * input, int input_stride, float * ptFeatures, uint8_t * output, int output_stride, float * bias, float * weights);
#ifdef ADM_CPU_HAS_SIMD
# ifdef ADM_CPU_HAS_X86_SIMD    
    static void     fsrcnn_feature_layer_8_AVX(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights);
    static void     fsrcnn_feature_layer_16_AVX(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights);
    static void     fsrcnn_feature_layer_16_FMA(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights);
    static void     fsrcnn_model_layer_8_AVX(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_model_layer_12_FMA(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_model_layer_16_AVX(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_model_layer_16_FMA(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_residual_layer_8_AVX(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_residual_layer_12_FMA(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_residual_layer_16_AVX(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_residual_layer_16_FMA(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_subconvolutional_layer_4x_16_FMA(int kernel_size, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights);
    static void     plxnet_feature_layer_2x_12_FMA(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights, float * output2, float * bias2, float * weights2);
# endif
    static void     fsrcnn_feature_layer_8_SSE(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights);
    static void     fsrcnn_feature_layer_16_SSE(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights);
    static void     fsrcnn_model_layer_8_SSE(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_model_layer_12_SSE(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_model_layer_16_SSE(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_residual_layer_8_SSE(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_residual_layer_12_SSE(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_residual_layer_16_SSE(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_subconvolutional_layer_2x_8_SSE(int kernel_size, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights);
    static void     fsrcnn_subconvolutional_layer_2x_16_SSE(int kernel_size, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights);
    static void     fsrcnn_subconvolutional_layer_3x_16_SSE(int kernel_size, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights);
    static void     fsrcnn_subconvolutional_layer_4x_16_SSE(int kernel_size, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights);
    static void     plxnet_feature_layer_2x_12_SSE(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights, float * output2, float * bias2, float * weights2);
    static void     plxnet_subconvolutional_layer_2x_12_SSE(int kernel_size, float * input, int input_stride, float * ptFeatures, uint8_t * output, int output_stride, float * bias, float * weights);
#endif    
    
    
protected:
    unsigned int            w,h,threads;

    static void     fsrcnn_feature_layer_8(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights);
    static void     fsrcnn_feature_layer_16(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights);
    static void     fsrcnn_model_layer_8(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_model_layer_12(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_model_layer_16(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_residual_layer_8(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_residual_layer_12(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_residual_layer_16(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha);
    static void     fsrcnn_subconvolutional_layer_8(int kernel_size, int scale, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights);
    static void     fsrcnn_subconvolutional_layer_16(int kernel_size, int scale, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights);

    static void     plxnet_feature_layer_12(int kernel_size, int scale, uint8_t * input, int input_stride, float * output, float * bias, float * weights, float * output2, float * bias2, float * weights2);
    static void     plxnet_subconvolutional_layer_12(int kernel_size, int scale, float * input, int input_stride, float * ptFeatures, uint8_t * output, int output_stride, float * bias, float * weights);
    
    static void     transposeWeights(int features, float * weights, int weightCount);
    static void     shuffleWeights(int features, float * weights, int weightCount);
    
public:
                            NeuronSW(int w, int h);
    virtual                 ~NeuronSW();
    virtual void            upscaleY(ADMImage *srcImg, ADMImage *dstImg) {};
        
};

