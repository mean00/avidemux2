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
#define _USE_MATH_DEFINES // some compilers do not export M_PI etc.. if GNU_SOURCE or that is defined, let's do that
#include <cmath>
#include "NeuronSW.h"


NeuronSW::NeuronSW(int w, int h)
{
    this->w = w;
    this->h = h;
    threads = ADM_cpu_num_processors();
    if (threads > 4)
        threads--;
    if (threads < 1)
        threads = 1;
}

NeuronSW::~NeuronSW()
{
    
}

#define MM_LOAD_8_BIAS_SSE()        t[0] = _mm_load_ps(bias + 0); \
                                    t[1] = _mm_load_ps(bias + 4);

#define MM_LOAD_16_BIAS_SSE()       t[0] = _mm_load_ps(bias + 0); \
                                    t[1] = _mm_load_ps(bias + 4); \
                                    t[2] = _mm_load_ps(bias + 8); \
                                    t[3] = _mm_load_ps(bias + 12);

#define MM_LOAD_8_BIAS_AVX()        t[0] = _mm256_loadu_ps(bias + 0);

#define MM_LOAD_16_BIAS_AVX()       t[0] = _mm256_loadu_ps(bias + 0); \
                                    t[1] = _mm256_loadu_ps(bias + 8);

#define MM_STORE_8_OUT_SSE()        _mm_store_ps(output + 0, t[0]); \
                                    _mm_store_ps(output + 4, t[1]);

#define MM_STORE_16_OUT_SSE()       _mm_store_ps(output + 0, t[0]); \
                                    _mm_store_ps(output + 4, t[1]); \
                                    _mm_store_ps(output + 8, t[2]); \
                                    _mm_store_ps(output + 12, t[3]);

#define MM_STORE_8_OUT_AVX()        _mm256_storeu_ps(output + 0, t[0]);

#define MM_STORE_16_OUT_AVX()       _mm256_storeu_ps(output + 0, t[0]); \
                                    _mm256_storeu_ps(output + 8, t[1]);

#define MM_ALPHA_8_AVX()            __m256 zero = _mm256_setzero_ps(); \
                                    __m256 alpha1 = _mm256_loadu_ps(alpha + 0); \
                                    t[0] = _mm256_add_ps(_mm256_max_ps(t[0], zero), _mm256_mul_ps(_mm256_min_ps(t[0], zero), alpha1));

#define MM_ALPHA_16_AVX()           __m256 zero = _mm256_setzero_ps(); \
                                    __m256 alpha1 = _mm256_loadu_ps(alpha + 0); \
                                    __m256 alpha2 = _mm256_loadu_ps(alpha + 8); \
                                    t[0] = _mm256_add_ps(_mm256_max_ps(t[0], zero), _mm256_mul_ps(_mm256_min_ps(t[0], zero), alpha1)); \
                                    t[1] = _mm256_add_ps(_mm256_max_ps(t[1], zero), _mm256_mul_ps(_mm256_min_ps(t[1], zero), alpha2));

#define MM_ALPHA_8_SSE()            __m128 zero = _mm_setzero_ps(); \
                                    __m128 alpha1 = _mm_load_ps(alpha + 0); \
                                    __m128 alpha2 = _mm_load_ps(alpha + 4); \
                                    t[0] = _mm_add_ps(_mm_max_ps(t[0], zero), _mm_mul_ps(_mm_min_ps(t[0], zero), alpha1)); \
                                    t[1] = _mm_add_ps(_mm_max_ps(t[1], zero), _mm_mul_ps(_mm_min_ps(t[1], zero), alpha2));

#define MM_ALPHA_16_SSE()           __m128 zero = _mm_setzero_ps(); \
                                    __m128 alpha1 = _mm_load_ps(alpha + 0); \
                                    __m128 alpha2 = _mm_load_ps(alpha + 4); \
                                    __m128 alpha3 = _mm_load_ps(alpha + 8); \
                                    __m128 alpha4 = _mm_load_ps(alpha + 12); \
                                    t[0] = _mm_add_ps(_mm_max_ps(t[0], zero), _mm_mul_ps(_mm_min_ps(t[0], zero), alpha1)); \
                                    t[1] = _mm_add_ps(_mm_max_ps(t[1], zero), _mm_mul_ps(_mm_min_ps(t[1], zero), alpha2)); \
                                    t[2] = _mm_add_ps(_mm_max_ps(t[2], zero), _mm_mul_ps(_mm_min_ps(t[2], zero), alpha3)); \
                                    t[3] = _mm_add_ps(_mm_max_ps(t[3], zero), _mm_mul_ps(_mm_min_ps(t[3], zero), alpha4));

#define MM_SSE2_LOAD_V(vec)         v4 = _mm_load_ps(vec);    vec += 4; \
                                    u1 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(0,0,0,0)); \
                                    u2 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(1,1,1,1)); \
                                    u3 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(2,2,2,2)); \
                                    u4 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(3,3,3,3));

#define MM_SSE2_MX_V_MULT(t, mx)    prod1 = _mm_mul_ps(_mm_load_ps(mx), u1);    mx+=4;\
                                    prod2 = _mm_mul_ps(_mm_load_ps(mx), u2);    mx+=4;\
                                    prod3 = _mm_mul_ps(_mm_load_ps(mx), u3);    mx+=4;\
                                    prod4 = _mm_mul_ps(_mm_load_ps(mx), u4);    mx+=4;\
                                    t = _mm_add_ps(t,_mm_add_ps(_mm_add_ps(prod1, prod2), _mm_add_ps(prod3, prod4)));

#define MM_FMA_4x_MX_V_MULT(mx)     t[0] = _mm_fmadd_ps(_mm_load_ps(mx),u1, t[0]);     mx += 16; \
                                    t[1] = _mm_fmadd_ps(_mm_load_ps(mx),u1, t[1]);     mx += 16; \
                                    t[2] = _mm_fmadd_ps(_mm_load_ps(mx),u1, t[2]);     mx += 16; \
                                    t[3] = _mm_fmadd_ps(_mm_load_ps(mx),u1, t[3]);     mx -= 44; \
                                    t[0] = _mm_fmadd_ps(_mm_load_ps(mx),u2, t[0]);     mx += 16; \
                                    t[1] = _mm_fmadd_ps(_mm_load_ps(mx),u2, t[1]);     mx += 16; \
                                    t[2] = _mm_fmadd_ps(_mm_load_ps(mx),u2, t[2]);     mx += 16; \
                                    t[3] = _mm_fmadd_ps(_mm_load_ps(mx),u2, t[3]);     mx -= 44; \
                                    t[0] = _mm_fmadd_ps(_mm_load_ps(mx),u3, t[0]);     mx += 16; \
                                    t[1] = _mm_fmadd_ps(_mm_load_ps(mx),u3, t[1]);     mx += 16; \
                                    t[2] = _mm_fmadd_ps(_mm_load_ps(mx),u3, t[2]);     mx += 16; \
                                    t[3] = _mm_fmadd_ps(_mm_load_ps(mx),u3, t[3]);     mx -= 44; \
                                    t[0] = _mm_fmadd_ps(_mm_load_ps(mx),u4, t[0]);     mx += 16; \
                                    t[1] = _mm_fmadd_ps(_mm_load_ps(mx),u4, t[1]);     mx += 16; \
                                    t[2] = _mm_fmadd_ps(_mm_load_ps(mx),u4, t[2]);     mx += 16; \
                                    t[3] = _mm_fmadd_ps(_mm_load_ps(mx),u4, t[3]);     mx += 4;

#define MM_AVX_2x_LOAD_V(vec)       v8 = _mm256_broadcast_ps(reinterpret_cast<__m128 *>(vec));  vec += 4; \
                                    u1 = _mm256_shuffle_ps(v8,v8, _MM_SHUFFLE(0,0,0,0)); \
                                    u2 = _mm256_shuffle_ps(v8,v8, _MM_SHUFFLE(1,1,1,1)); \
                                    u3 = _mm256_shuffle_ps(v8,v8, _MM_SHUFFLE(2,2,2,2)); \
                                    u4 = _mm256_shuffle_ps(v8,v8, _MM_SHUFFLE(3,3,3,3));

#define MM_AVX_2x_MX_V_MULT(t, mx)  prod1 = _mm256_mul_ps(_mm256_loadu_ps(mx), u1);    mx += 8; \
                                    prod2 = _mm256_mul_ps(_mm256_loadu_ps(mx), u2);    mx += 8; \
                                    prod3 = _mm256_mul_ps(_mm256_loadu_ps(mx), u3);    mx += 8; \
                                    prod4 = _mm256_mul_ps(_mm256_loadu_ps(mx), u4);    mx += 8; \
                                    t = _mm256_add_ps(t,_mm256_add_ps(_mm256_add_ps(prod1, prod2), _mm256_add_ps(prod3, prod4)));



void NeuronSW::fsrcnn_feature_layer_C(int features, int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights)
{
    float * t = (float*)admAlloca(sizeof(float) * features);
    int radius = kernel_size/2;
    float pix;
    
    for (int i=0; i<features; i++)
        t[i] = bias[i];
    
    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            pix = *(input + q*input_stride + p) / 255.0;
            for (int i=0; i<features; i++)
            {
                t[i] += pix * *weights;
                weights++;
            }
        }
    }
    
    for (int i=0; i<features; i++)
    {
        *output = t[i];
        output++;
    }  
}

#ifdef ADM_CPU_HAS_SIMD
# ifdef ADM_CPU_HAS_X86_SIMD
ADM_CPU_X86_SIMD_TARGET("avx")
void NeuronSW::fsrcnn_feature_layer_8_AVX(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights)
{
    __m256 t[1];
    int radius = kernel_size/2;
    float pix;

    MM_LOAD_8_BIAS_AVX();
    
    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            pix = *(input + q*input_stride + p) / 255.0;
            __m256 s = _mm256_broadcast_ss(&pix);
            __m256 v1 = _mm256_loadu_ps(weights);   weights += 8;
            t[0] = _mm256_add_ps(t[0], _mm256_mul_ps(v1, s));
        }
    }
    
    MM_STORE_8_OUT_AVX();
}
# endif

void NeuronSW::fsrcnn_feature_layer_8_SSE(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights)
{
    __m128 t[2];
    int radius = kernel_size/2;
    float pix;

    MM_LOAD_8_BIAS_SSE();
    
    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            pix = *(input + q*input_stride + p) / 255.0;
            __m128 s = _mm_load1_ps(&pix);
            __m128 v1 = _mm_load_ps(weights);   weights += 4;
            __m128 v2 = _mm_load_ps(weights);   weights += 4;
            t[0] = _mm_add_ps(t[0], _mm_mul_ps(v1, s));
            t[1] = _mm_add_ps(t[1], _mm_mul_ps(v2, s));            
        }
    }
    
    MM_STORE_8_OUT_SSE();
}

# ifdef ADM_CPU_HAS_X86_SIMD
ADM_CPU_X86_SIMD_TARGET("avx")
void NeuronSW::fsrcnn_feature_layer_16_AVX(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights)
{
    __m256 t[2];
    int radius = kernel_size/2;
    float pix;

    MM_LOAD_16_BIAS_AVX();
    
    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            pix = *(input + q*input_stride + p) / 255.0;
            __m256 s = _mm256_broadcast_ss(&pix);
            __m256 v1 = _mm256_loadu_ps(weights);   weights += 8;
            __m256 v2 = _mm256_loadu_ps(weights);   weights += 8;
            t[0] = _mm256_add_ps(t[0], _mm256_mul_ps(v1, s));
            t[1] = _mm256_add_ps(t[1], _mm256_mul_ps(v2, s));
        }
    }
    
    MM_STORE_16_OUT_AVX();
}

ADM_CPU_X86_SIMD_TARGET("fma")
void NeuronSW::fsrcnn_feature_layer_16_FMA(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights)
{
    __m128 t[4];
    int radius = kernel_size/2;
    float pix;

    MM_LOAD_16_BIAS_SSE();

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            pix = *(input + q*input_stride + p) / 255.0;

            __m128 s = _mm_load1_ps(&pix);
            __m128 v1 = _mm_load_ps(weights);   weights += 4;
            __m128 v2 = _mm_load_ps(weights);   weights += 4;
            __m128 v3 = _mm_load_ps(weights);   weights += 4;
            __m128 v4 = _mm_load_ps(weights);   weights += 4;
            t[0] = _mm_fmadd_ps(v1, s, t[0]);
            t[1] = _mm_fmadd_ps(v2, s, t[1]);
            t[2] = _mm_fmadd_ps(v3, s, t[2]);
            t[3] = _mm_fmadd_ps(v4, s, t[3]);
        }
    }
    
    MM_STORE_16_OUT_SSE();
}
# endif

void NeuronSW::fsrcnn_feature_layer_16_SSE(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights)
{
    __m128 t[4];
    int radius = kernel_size/2;
    float pix;

    MM_LOAD_16_BIAS_SSE();

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            pix = *(input + q*input_stride + p) / 255.0;

            __m128 s = _mm_load1_ps(&pix);
            __m128 v1 = _mm_load_ps(weights);   weights += 4;
            __m128 v2 = _mm_load_ps(weights);   weights += 4;
            __m128 v3 = _mm_load_ps(weights);   weights += 4;
            __m128 v4 = _mm_load_ps(weights);   weights += 4;
            t[0] = _mm_add_ps(t[0], _mm_mul_ps(v1, s));
            t[1] = _mm_add_ps(t[1], _mm_mul_ps(v2, s));
            t[2] = _mm_add_ps(t[2], _mm_mul_ps(v3, s));
            t[3] = _mm_add_ps(t[3], _mm_mul_ps(v4, s));            
        }
    }
    
    MM_STORE_16_OUT_SSE();
}

#endif    


void NeuronSW::fsrcnn_feature_layer_8(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights)
{
#ifdef ADM_CPU_HAS_SIMD
# ifdef ADM_CPU_HAS_X86_SIMD
    if (CpuCaps::hasAVX()) return fsrcnn_feature_layer_8_AVX(kernel_size, input, input_stride, output, bias, weights);
# endif
    return fsrcnn_feature_layer_8_SSE(kernel_size, input, input_stride, output, bias, weights);
#endif
    return fsrcnn_feature_layer_C(8, kernel_size, input, input_stride, output, bias, weights);
}


void NeuronSW::fsrcnn_feature_layer_16(int kernel_size, uint8_t * input, int input_stride, float * output, float * bias, float * weights)
{
#ifdef ADM_CPU_HAS_SIMD
# ifdef ADM_CPU_HAS_X86_SIMD
    if (CpuCaps::isAMD() && CpuCaps::hasFMA3()) return fsrcnn_feature_layer_16_FMA(kernel_size, input, input_stride, output, bias, weights);
    if (CpuCaps::hasAVX()) return fsrcnn_feature_layer_16_AVX(kernel_size, input, input_stride, output, bias, weights);
# endif
    return fsrcnn_feature_layer_16_SSE(kernel_size, input, input_stride, output, bias, weights);
#endif    
    return fsrcnn_feature_layer_C(16, kernel_size, input, input_stride, output, bias, weights);
}



void NeuronSW::fsrcnn_model_layer_C(int features, int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha)
{
    float * t = (float*)admAlloca(sizeof(float) * features);
    int radius = kernel_size/2;
    
    for (int i=0; i<features; i++)
        t[i] = bias[i];
    
    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*features;
            for (int i=0; i<(features/4); i++)
            {
                for (int m=0; m<(features/4); m++)
                {
                    for (int j=0; j<4; j++)
                    {
                        float sum = 0;
                        for (int k=0; k<4; k++)
                        {
                            sum += *weights * vec[k];
                            weights++;
                        }
                        t[m*4+j] += sum;
                    }
                }
                vec += 4;
            }
        }
    }

    for (int i=0; i<features; i++)
    {
        t[i] = ((t[i] < 0) ? 0.0 : t[i]) + alpha[i]*((t[i] > 0) ? 0.0 : t[i]);
    }
    
    for (int i=0; i<features; i++)
    {
        *output = t[i];
        output++;
    }      
}

#ifdef ADM_CPU_HAS_SIMD
# ifdef ADM_CPU_HAS_X86_SIMD
ADM_CPU_X86_SIMD_TARGET("avx")
void NeuronSW::fsrcnn_model_layer_8_AVX(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha)
{
    __m256 t[1];
    int radius = kernel_size/2;

    MM_LOAD_8_BIAS_AVX();
    
    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*8;
            __m256 v8, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
        }
    }
 
    MM_ALPHA_8_AVX();
    
    MM_STORE_8_OUT_AVX();
}
# endif

void NeuronSW::fsrcnn_model_layer_8_SSE(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha)
{
    __m128 t[2];
    int radius = kernel_size/2;

    MM_LOAD_8_BIAS_SSE();

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*8;
            __m128 v4, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
            MM_SSE2_LOAD_V(vec);
            MM_SSE2_MX_V_MULT(t[0], weights);
            MM_SSE2_MX_V_MULT(t[1], weights);
            MM_SSE2_LOAD_V(vec);
            MM_SSE2_MX_V_MULT(t[0], weights);
            MM_SSE2_MX_V_MULT(t[1], weights);
        }
    }

    MM_ALPHA_8_SSE();
    
    MM_STORE_8_OUT_SSE();
}


# ifdef ADM_CPU_HAS_X86_SIMD
ADM_CPU_X86_SIMD_TARGET("avx")
void NeuronSW::fsrcnn_model_layer_16_AVX(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha)
{
    __m256 t[2];
    int radius = kernel_size/2;

    MM_LOAD_16_BIAS_AVX();
    
    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            __m256 v8, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
            MM_AVX_2x_MX_V_MULT(t[1], weights);
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
            MM_AVX_2x_MX_V_MULT(t[1], weights);
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
            MM_AVX_2x_MX_V_MULT(t[1], weights);
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
            MM_AVX_2x_MX_V_MULT(t[1], weights);
        }
    }
 
    MM_ALPHA_16_AVX();
    
    MM_STORE_16_OUT_AVX();
}

ADM_CPU_X86_SIMD_TARGET("fma")
void NeuronSW::fsrcnn_model_layer_16_FMA(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha)
{
    __m128 t[4];
    int radius = kernel_size/2;

    MM_LOAD_16_BIAS_SSE();

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            __m128 v4, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
            for (int i=0; i<4; i++)
            {
                MM_SSE2_LOAD_V(vec);
                MM_FMA_4x_MX_V_MULT(weights);
            }

        }
    }

    MM_ALPHA_16_SSE();
    
    MM_STORE_16_OUT_SSE();
}
# endif

void NeuronSW::fsrcnn_model_layer_16_SSE(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha)
{
    __m128 t[4];
    int radius = kernel_size/2;

    MM_LOAD_16_BIAS_SSE();

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            for (int i=0; i<4; i++)
            {
                __m128 v4, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
                MM_SSE2_LOAD_V(vec);
                MM_SSE2_MX_V_MULT(t[0], weights);
                MM_SSE2_MX_V_MULT(t[1], weights);
                MM_SSE2_MX_V_MULT(t[2], weights);
                MM_SSE2_MX_V_MULT(t[3], weights);
            }            
        }
    }

    MM_ALPHA_16_SSE();
    
    MM_STORE_16_OUT_SSE();
}
#endif

void NeuronSW::fsrcnn_model_layer_8(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha)
{
#ifdef ADM_CPU_HAS_SIMD
# ifdef ADM_CPU_HAS_X86_SIMD
    if (CpuCaps::hasAVX()) return fsrcnn_model_layer_8_AVX(kernel_size, input, input_stride, output, bias, weights, alpha);
# endif
    return fsrcnn_model_layer_8_SSE(kernel_size, input, input_stride, output, bias, weights, alpha);
#endif    
    return fsrcnn_model_layer_C(8, kernel_size, input, input_stride, output, bias, weights, alpha);
}

void NeuronSW::fsrcnn_model_layer_16(int kernel_size, float * input, int input_stride, float * output, float * bias, float * weights, float * alpha)
{
#ifdef ADM_CPU_HAS_SIMD
# ifdef ADM_CPU_HAS_X86_SIMD
    if (CpuCaps::isAMD() && CpuCaps::hasFMA3()) return fsrcnn_model_layer_16_FMA(kernel_size, input, input_stride, output, bias, weights, alpha);
    if (CpuCaps::hasAVX()) return fsrcnn_model_layer_16_AVX(kernel_size, input, input_stride, output, bias, weights, alpha);
# endif
    return fsrcnn_model_layer_16_SSE(kernel_size, input, input_stride, output, bias, weights, alpha);
#endif    
    return fsrcnn_model_layer_C(16, kernel_size, input, input_stride, output, bias, weights, alpha);
}



void NeuronSW::fsrcnn_residual_layer_C(int features, int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha)
{
    float * t = (float*)admAlloca(sizeof(float) * features);
    int radius = kernel_size/2;
    
    for (int i=0; i<features; i++)
        t[i] = bias[i];
    
    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*features;
            for (int i=0; i<(features/4); i++)
            {
                for (int m=0; m<(features/4); m++)
                {
                    for (int j=0; j<4; j++)
                    {
                        float sum = 0;
                        for (int k=0; k<4; k++)
                        {
                            sum += *weights * vec[k];
                            weights++;
                        }
                        t[m*4+j] += sum;
                    }
                }
                vec += 4;
            }
        }
    }

    for (int i=0; i<features; i++)
    {
        t[i] += residual[i];
    }
    
    for (int i=0; i<features; i++)
    {
        t[i] = ((t[i] < 0) ? 0.0 : t[i]) + alpha[i]*((t[i] > 0) ? 0.0 : t[i]);
    }
    
    for (int i=0; i<features; i++)
    {
        *output = t[i];
        output++;
    }    
}

#ifdef ADM_CPU_HAS_SIMD
# ifdef ADM_CPU_HAS_X86_SIMD
ADM_CPU_X86_SIMD_TARGET("avx")
void NeuronSW::fsrcnn_residual_layer_8_AVX(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha)
{
    __m256 t[1];
    int radius = kernel_size/2;

    MM_LOAD_8_BIAS_AVX();
    
    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            __m256 v8, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
        }
    }
 
    t[0] = _mm256_add_ps(t[0], _mm256_loadu_ps(residual + 0));
    
    MM_ALPHA_8_AVX();
    
    MM_STORE_8_OUT_AVX();
}
# endif

void NeuronSW::fsrcnn_residual_layer_8_SSE(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha)
{
    __m128 t[2];
    int radius = kernel_size/2;

    MM_LOAD_8_BIAS_SSE();

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            __m128 v4, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
            MM_SSE2_LOAD_V(vec);
            MM_SSE2_MX_V_MULT(t[0], weights);
            MM_SSE2_MX_V_MULT(t[1], weights);
            MM_SSE2_LOAD_V(vec);
            MM_SSE2_MX_V_MULT(t[0], weights);
            MM_SSE2_MX_V_MULT(t[1], weights);
        }
    }
    
    t[0] = _mm_add_ps(t[0], _mm_load_ps(residual + 0));
    t[1] = _mm_add_ps(t[1], _mm_load_ps(residual + 4));
    
    MM_ALPHA_8_SSE();
    
    MM_STORE_8_OUT_SSE();
}


# ifdef ADM_CPU_HAS_X86_SIMD
ADM_CPU_X86_SIMD_TARGET("avx")
void NeuronSW::fsrcnn_residual_layer_16_AVX(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha)
{
    __m256 t[2];
    int radius = kernel_size/2;

    MM_LOAD_16_BIAS_AVX();
    
    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            __m256 v8, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
            MM_AVX_2x_MX_V_MULT(t[1], weights);
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
            MM_AVX_2x_MX_V_MULT(t[1], weights);
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
            MM_AVX_2x_MX_V_MULT(t[1], weights);
            MM_AVX_2x_LOAD_V(vec);
            MM_AVX_2x_MX_V_MULT(t[0], weights);
            MM_AVX_2x_MX_V_MULT(t[1], weights);
        }
    }
 
    t[0] = _mm256_add_ps(t[0], _mm256_loadu_ps(residual + 0));
    t[1] = _mm256_add_ps(t[1], _mm256_loadu_ps(residual + 8));
    
    MM_ALPHA_16_AVX();
    
    MM_STORE_16_OUT_AVX();
}

ADM_CPU_X86_SIMD_TARGET("fma")
void NeuronSW::fsrcnn_residual_layer_16_FMA(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha)
{
    __m128 t[4];
    int radius = kernel_size/2;

    MM_LOAD_16_BIAS_SSE();

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            __m128 v4, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
            for (int i=0; i<4; i++)
            {
                MM_SSE2_LOAD_V(vec);
                MM_FMA_4x_MX_V_MULT(weights);
            }

        }
    }

    t[0] = _mm_add_ps(t[0], _mm_load_ps(residual + 0));
    t[1] = _mm_add_ps(t[1], _mm_load_ps(residual + 4));
    t[2] = _mm_add_ps(t[2], _mm_load_ps(residual + 8));
    t[3] = _mm_add_ps(t[3], _mm_load_ps(residual + 12));
    
    MM_ALPHA_16_SSE();
    
    MM_STORE_16_OUT_SSE();
}
# endif

void NeuronSW::fsrcnn_residual_layer_16_SSE(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha)
{
    __m128 t[4];
    int radius = kernel_size/2;

    MM_LOAD_16_BIAS_SSE();

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            for (int i=0; i<4; i++)
            {
                __m128 v4, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
                MM_SSE2_LOAD_V(vec);
                MM_SSE2_MX_V_MULT(t[0], weights);
                MM_SSE2_MX_V_MULT(t[1], weights);
                MM_SSE2_MX_V_MULT(t[2], weights);
                MM_SSE2_MX_V_MULT(t[3], weights);
            }            
        }
    }
    
    t[0] = _mm_add_ps(t[0], _mm_load_ps(residual + 0));
    t[1] = _mm_add_ps(t[1], _mm_load_ps(residual + 4));
    t[2] = _mm_add_ps(t[2], _mm_load_ps(residual + 8));
    t[3] = _mm_add_ps(t[3], _mm_load_ps(residual + 12));
    
    MM_ALPHA_16_SSE();
    
    MM_STORE_16_OUT_SSE();
}
#endif

void NeuronSW::fsrcnn_residual_layer_8(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha)
{
#ifdef ADM_CPU_HAS_SIMD
# ifdef ADM_CPU_HAS_X86_SIMD
    if (CpuCaps::hasAVX()) return fsrcnn_residual_layer_8_AVX(kernel_size, input, input_stride, residual, output, bias, weights, alpha);
# endif
    return fsrcnn_residual_layer_8_SSE(kernel_size, input, input_stride, residual, output, bias, weights, alpha);
#endif    
    return fsrcnn_residual_layer_C(8, kernel_size, input, input_stride, residual, output, bias, weights, alpha);
}



void NeuronSW::fsrcnn_residual_layer_16(int kernel_size, float * input, int input_stride, float * residual, float * output, float * bias, float * weights, float * alpha)
{
#ifdef ADM_CPU_HAS_SIMD
# ifdef ADM_CPU_HAS_X86_SIMD
    if (CpuCaps::isAMD() && CpuCaps::hasFMA3()) return fsrcnn_residual_layer_16_FMA(kernel_size, input, input_stride, residual, output, bias, weights, alpha);
    if (CpuCaps::hasAVX()) return fsrcnn_residual_layer_16_AVX(kernel_size, input, input_stride, residual, output, bias, weights, alpha);
# endif
    return fsrcnn_residual_layer_16_SSE(kernel_size, input, input_stride, residual, output, bias, weights, alpha);
#endif    
    return fsrcnn_residual_layer_C(16, kernel_size, input, input_stride, residual, output, bias, weights, alpha);
}


void NeuronSW::fsrcnn_subconvolutional_layer_C(int features, int kernel_size, int scale, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights)
{
    float * t = (float*)admAlloca(sizeof(float) * features);
    int radius = kernel_size/2;
    int scs = scale;
    if (scs == 2)
        scs = 1;
    
    for (int i=0; i<4*scs; i++)
        t[i] = bias[i];
    
    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*features;
            for (int i=0; i<(features/4); i++)
            {
                for (int m=0; m<scs; m++)
                {
                    for (int j=0; j<4; j++)
                    {
                        float sum = 0;
                        for (int k=0; k<4; k++)
                        {
                            sum += *weights * vec[k];
                            weights++;
                        }
                        t[m*4+j] += sum;
                    }
                }
                vec += 4;
            }
        }
    }

    for (int i=0; i<4*scs; i++)
    {
        t[i] *= 255.0;
        t[i] = std::round(t[i]);
        if (t[i] < 0) t[i] = 0;
        if (t[i] > 255) t[i] = 255;
    }
    for (int i=0; i<scale; i++)
    {
        for (int j=0; j<scale; j++)
        {
            if (scale > 2)
                output[j*output_stride + i] = t[i*4+j];
            else
                output[j*output_stride + i] = t[i*2+j];
        }
    }    
}

#ifdef ADM_CPU_HAS_SIMD
void NeuronSW::fsrcnn_subconvolutional_layer_2x_8_SSE(int kernel_size, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights)
{
    __m128 t[1];
    int radius = kernel_size/2;

    t[0] = _mm_load_ps(bias + 0);

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*8;
            __m128 v4, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
            MM_SSE2_LOAD_V(vec);
            MM_SSE2_MX_V_MULT(t[0], weights);
            MM_SSE2_LOAD_V(vec);
            MM_SSE2_MX_V_MULT(t[0], weights);
        }
    }

    float c255 = 255;
    __m128 zero = _mm_setzero_ps();
    __m128 v255 = _mm_load1_ps(&c255);
    t[0] = _mm_mul_ps(t[0], v255);
    t[0] = _mm_max_ps(t[0], zero);
    t[0] = _mm_min_ps(t[0], v255);
    __m128i intvalue;
    uint32_t pixels;
    uint8_t * pix = ((uint8_t*)&pixels);
    intvalue = _mm_cvtps_epi32(t[0]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0+0] = pix[0];
    output[0+1] = pix[2];
    output[output_stride+0] = pix[1];
    output[output_stride+1] = pix[3];    
}

void NeuronSW::fsrcnn_subconvolutional_layer_2x_16_SSE(int kernel_size, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights)
{
    __m128 t[1];
    int radius = kernel_size/2;

    t[0] = _mm_load_ps(bias + 0);

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            __m128 v4, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
            MM_SSE2_LOAD_V(vec);
            MM_SSE2_MX_V_MULT(t[0], weights);
            MM_SSE2_LOAD_V(vec);
            MM_SSE2_MX_V_MULT(t[0], weights);
            MM_SSE2_LOAD_V(vec);
            MM_SSE2_MX_V_MULT(t[0], weights);
            MM_SSE2_LOAD_V(vec);
            MM_SSE2_MX_V_MULT(t[0], weights);
        }
    }

    float c255 = 255;
    __m128 zero = _mm_setzero_ps();
    __m128 v255 = _mm_load1_ps(&c255);
    t[0] = _mm_mul_ps(t[0], v255);
    t[0] = _mm_max_ps(t[0], zero);
    t[0] = _mm_min_ps(t[0], v255);
    __m128i intvalue;
    uint32_t pixels;
    uint8_t * pix = ((uint8_t*)&pixels);
    intvalue = _mm_cvtps_epi32(t[0]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0+0] = pix[0];
    output[0+1] = pix[2];
    output[output_stride+0] = pix[1];
    output[output_stride+1] = pix[3];
}

void NeuronSW::fsrcnn_subconvolutional_layer_3x_16_SSE(int kernel_size, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights)
{
    __m128 t[3];
    int radius = kernel_size/2;

    t[0] = _mm_load_ps(bias + 0);
    t[1] = _mm_load_ps(bias + 4);
    t[2] = _mm_load_ps(bias + 8);

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            for (int i=0; i<4; i++)
            {
                __m128 v4, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
                MM_SSE2_LOAD_V(vec);
                MM_SSE2_MX_V_MULT(t[0], weights);
                MM_SSE2_MX_V_MULT(t[1], weights);
                MM_SSE2_MX_V_MULT(t[2], weights);
            }  
        }
    }    
    
    float c255 = 255;
    __m128 zero = _mm_setzero_ps();
    __m128 v255 = _mm_load1_ps(&c255);
    t[0] = _mm_mul_ps(t[0], v255);
    t[0] = _mm_max_ps(t[0], zero);
    t[0] = _mm_min_ps(t[0], v255);
    t[1] = _mm_mul_ps(t[1], v255);
    t[1] = _mm_max_ps(t[1], zero);
    t[1] = _mm_min_ps(t[1], v255);
    t[2] = _mm_mul_ps(t[2], v255);
    t[2] = _mm_max_ps(t[2], zero);
    t[2] = _mm_min_ps(t[2], v255);
    __m128i intvalue;
    uint32_t pixels;
    uint8_t * pix = ((uint8_t*)&pixels);
    intvalue = _mm_cvtps_epi32(t[0]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0+0] = pix[0];
    output[output_stride+0] = pix[1];
    output[2*output_stride+0] = pix[2];
    intvalue = _mm_cvtps_epi32(t[1]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0+1] = pix[0];
    output[output_stride+1] = pix[1];
    output[2*output_stride+1] = pix[2];
    intvalue = _mm_cvtps_epi32(t[2]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0+2] = pix[0];
    output[output_stride+2] = pix[1];
    output[2*output_stride+2] = pix[2];
}

# ifdef ADM_CPU_HAS_X86_SIMD
ADM_CPU_X86_SIMD_TARGET("fma")
void NeuronSW::fsrcnn_subconvolutional_layer_4x_16_FMA(int kernel_size, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights)
{
    __m128 t[4];
    int radius = kernel_size/2;

    t[0] = _mm_load_ps(bias + 0);
    t[1] = _mm_load_ps(bias + 4);
    t[2] = _mm_load_ps(bias + 8);
    t[3] = _mm_load_ps(bias + 12);

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            __m128 v4, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
            for (int i=0; i<4; i++)
            {
                MM_SSE2_LOAD_V(vec);
                MM_FMA_4x_MX_V_MULT(weights);
            }            
        }
    }

    float c255 = 255;
    __m128 zero = _mm_setzero_ps();
    __m128 v255 = _mm_load1_ps(&c255);
    t[0] = _mm_mul_ps(t[0], v255);
    t[0] = _mm_max_ps(t[0], zero);
    t[0] = _mm_min_ps(t[0], v255);
    t[1] = _mm_mul_ps(t[1], v255);
    t[1] = _mm_max_ps(t[1], zero);
    t[1] = _mm_min_ps(t[1], v255);
    t[2] = _mm_mul_ps(t[2], v255);
    t[2] = _mm_max_ps(t[2], zero);
    t[2] = _mm_min_ps(t[2], v255);
    t[3] = _mm_mul_ps(t[3], v255);
    t[3] = _mm_max_ps(t[3], zero);
    t[3] = _mm_min_ps(t[3], v255);
    __m128i intvalue;
    uint32_t pixels;
    uint8_t * pix = ((uint8_t*)&pixels);
    intvalue = _mm_cvtps_epi32(t[0]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0*output_stride+0] = pix[0];
    output[1*output_stride+0] = pix[1];
    output[2*output_stride+0] = pix[2];
    output[3*output_stride+0] = pix[3];
    intvalue = _mm_cvtps_epi32(t[1]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0*output_stride+1] = pix[0];
    output[1*output_stride+1] = pix[1];
    output[2*output_stride+1] = pix[2];
    output[3*output_stride+1] = pix[3];
    intvalue = _mm_cvtps_epi32(t[2]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0*output_stride+2] = pix[0];
    output[1*output_stride+2] = pix[1];
    output[2*output_stride+2] = pix[2];
    output[3*output_stride+2] = pix[3];
    intvalue = _mm_cvtps_epi32(t[3]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0*output_stride+3] = pix[0];
    output[1*output_stride+3] = pix[1];
    output[2*output_stride+3] = pix[2];
    output[3*output_stride+3] = pix[3];
}
# endif

void NeuronSW::fsrcnn_subconvolutional_layer_4x_16_SSE(int kernel_size, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights)
{
    __m128 t[4];
    int radius = kernel_size/2;

    t[0] = _mm_load_ps(bias + 0);
    t[1] = _mm_load_ps(bias + 4);
    t[2] = _mm_load_ps(bias + 8);
    t[3] = _mm_load_ps(bias + 12);

    for (int p=-radius; p<=radius; p++)
    {
        for (int q=-radius; q<=radius; q++)
        {
            float * vec = input + q*input_stride + p*16;
            for (int i=0; i<4; i++)
            {
                __m128 v4, u1, u2, u3, u4, prod1, prod2, prod3, prod4;
                MM_SSE2_LOAD_V(vec);
                MM_SSE2_MX_V_MULT(t[0], weights);
                MM_SSE2_MX_V_MULT(t[1], weights);
                MM_SSE2_MX_V_MULT(t[2], weights);
                MM_SSE2_MX_V_MULT(t[3], weights);
            }  
        }
    }

    float c255 = 255;
    __m128 zero = _mm_setzero_ps();
    __m128 v255 = _mm_load1_ps(&c255);
    t[0] = _mm_mul_ps(t[0], v255);
    t[0] = _mm_max_ps(t[0], zero);
    t[0] = _mm_min_ps(t[0], v255);
    t[1] = _mm_mul_ps(t[1], v255);
    t[1] = _mm_max_ps(t[1], zero);
    t[1] = _mm_min_ps(t[1], v255);
    t[2] = _mm_mul_ps(t[2], v255);
    t[2] = _mm_max_ps(t[2], zero);
    t[2] = _mm_min_ps(t[2], v255);
    t[3] = _mm_mul_ps(t[3], v255);
    t[3] = _mm_max_ps(t[3], zero);
    t[3] = _mm_min_ps(t[3], v255);
    __m128i intvalue;
    uint32_t pixels;
    uint8_t * pix = ((uint8_t*)&pixels);
    intvalue = _mm_cvtps_epi32(t[0]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0*output_stride+0] = pix[0];
    output[1*output_stride+0] = pix[1];
    output[2*output_stride+0] = pix[2];
    output[3*output_stride+0] = pix[3];
    intvalue = _mm_cvtps_epi32(t[1]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0*output_stride+1] = pix[0];
    output[1*output_stride+1] = pix[1];
    output[2*output_stride+1] = pix[2];
    output[3*output_stride+1] = pix[3];
    intvalue = _mm_cvtps_epi32(t[2]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0*output_stride+2] = pix[0];
    output[1*output_stride+2] = pix[1];
    output[2*output_stride+2] = pix[2];
    output[3*output_stride+2] = pix[3];
    intvalue = _mm_cvtps_epi32(t[3]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    output[0*output_stride+3] = pix[0];
    output[1*output_stride+3] = pix[1];
    output[2*output_stride+3] = pix[2];
    output[3*output_stride+3] = pix[3];
}
#endif

void NeuronSW::fsrcnn_subconvolutional_layer_8(int kernel_size, int scale, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights)
{
#ifdef ADM_CPU_HAS_SIMD
    if (scale == 2)
    {
        return fsrcnn_subconvolutional_layer_2x_8_SSE(kernel_size, input, input_stride, output, output_stride, bias, weights);
    }
#endif
    return fsrcnn_subconvolutional_layer_C(8, kernel_size, scale, input, input_stride, output, output_stride, bias, weights);
}


void NeuronSW::fsrcnn_subconvolutional_layer_16(int kernel_size, int scale, float * input, int input_stride, uint8_t * output, int output_stride, float * bias, float * weights)
{
#ifdef ADM_CPU_HAS_SIMD
    if (scale == 2)
    {
        return fsrcnn_subconvolutional_layer_2x_16_SSE(kernel_size, input, input_stride, output, output_stride, bias, weights);
    }
    if (scale == 3)
    {
        return fsrcnn_subconvolutional_layer_3x_16_SSE(kernel_size, input, input_stride, output, output_stride, bias, weights);
    }
    if (scale == 4)
    {
# ifdef ADM_CPU_HAS_X86_SIMD
        if (CpuCaps::isAMD() && CpuCaps::hasFMA3()) return fsrcnn_subconvolutional_layer_4x_16_FMA(kernel_size, input, input_stride, output, output_stride, bias, weights);
# endif        
        return fsrcnn_subconvolutional_layer_4x_16_SSE(kernel_size, input, input_stride, output, output_stride, bias, weights);
    }
#endif
    return fsrcnn_subconvolutional_layer_C(16, kernel_size, scale, input, input_stride, output, output_stride, bias, weights);
}







void NeuronSW::transposeWeights(int features, float * weights, int weightCount)
{
#ifdef ADM_CPU_HAS_SIMD
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

void NeuronSW::shuffleWeights(int features, float * weights, int weightCount)
{
#ifdef ADM_CPU_HAS_SIMD
# ifdef ADM_CPU_HAS_X86_SIMD
    if (CpuCaps::isAMD() && CpuCaps::hasFMA3())
    {
        if (features==16) return;   // SSE+FMA faster than AVX on 16 feature
    }
    if (CpuCaps::hasAVX())
    {
        float tmp[32];
        for (int k=0; k<(weightCount/32); k++)
        {
            for (int i=0; i<4; i++)
            {
                for (int j=0; j<4; j++)
                {
                    tmp[i*8 + 0 + j] = weights[i*4 + j];
                    tmp[i*8 + 4 + j] = weights[16 + i*4 + j];
                }
            }
            for (int i=0; i<32; i++)
            {
                weights[i] = tmp[i];
            }
            weights += 32;
        }
    }
# endif
#endif    
}