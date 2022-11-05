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
    if (threads < 1)
        threads = 1;
}

NeuronSW::~NeuronSW()
{
    
}

void NeuronSW::m_load1_bias(m_vec_base_t * t, float * bias)
{
#ifdef USE_SSE2
    t[0] = _mm_load_ps(bias);
#else    
    memcpy(t, bias, 4*1*sizeof(float));
#endif 
}


void NeuronSW::m_load2_bias(m_vec_base_t * t, float * bias)
{
#ifdef USE_SSE2
    t[0] = _mm_load_ps(bias + 0);
    t[1] = _mm_load_ps(bias + 4);
#else    
    memcpy(t, bias, 4*2*sizeof(float));
#endif 
}


void NeuronSW::m_load3_bias(m_vec_base_t * t, float * bias)
{
#ifdef USE_SSE2
    t[0] = _mm_load_ps(bias + 0);
    t[1] = _mm_load_ps(bias + 4);
    t[2] = _mm_load_ps(bias + 8);
#else    
    memcpy(t, bias, 4*3*sizeof(float));
#endif 
}

   
void NeuronSW::m_load4_bias(m_vec_base_t * t, float * bias)
{
#ifdef USE_SSE2
    t[0] = _mm_load_ps(bias + 0);
    t[1] = _mm_load_ps(bias + 4);
    t[2] = _mm_load_ps(bias + 8);
    t[3] = _mm_load_ps(bias + 12);
#else    
    memcpy(t, bias, 4*4*sizeof(float));
#endif 
}


void NeuronSW::m_add2_vec(m_vec_base_t * t, float * vec)
{
#ifdef USE_SSE2
    __m128 v1 = _mm_load_ps(vec + 0);
    __m128 v2 = _mm_load_ps(vec + 4);
    t[0] = _mm_add_ps(t[0], v1);
    t[1] = _mm_add_ps(t[1], v2);
#else    
    for (int i=0; i<4*2; i++)
    {
        t[i] += vec[i];
    }
#endif 
}


void NeuronSW::m_add4_vec(m_vec_base_t * t, float * vec)
{
#ifdef USE_SSE2
    __m128 v1 = _mm_load_ps(vec + 0);
    __m128 v2 = _mm_load_ps(vec + 4);
    __m128 v3 = _mm_load_ps(vec + 8);
    __m128 v4 = _mm_load_ps(vec + 12);
    t[0] = _mm_add_ps(t[0], v1);
    t[1] = _mm_add_ps(t[1], v2);
    t[2] = _mm_add_ps(t[2], v3);
    t[3] = _mm_add_ps(t[3], v4);
#else    
    for (int i=0; i<4*4; i++)
    {
        t[i] += vec[i];
    }
#endif 
}


void NeuronSW::m_add2_vecXs(m_vec_base_t * t, float * vec, float scalar)
{
#ifdef USE_SSE2
    __m128 s = _mm_load1_ps(&scalar);
    __m128 v1 = _mm_load_ps(vec + 0);
    __m128 v2 = _mm_load_ps(vec + 4);
    t[0] = _mm_add_ps(t[0], _mm_mul_ps(v1, s));
    t[1] = _mm_add_ps(t[1], _mm_mul_ps(v2, s));
#else    
    for (int i=0; i<4*2; i++)
    {
        t[i] += (vec[i] * scalar);
    }
#endif 
}


void NeuronSW::m_add4_vecXs(m_vec_base_t * t, float * vec, float scalar)
{
#ifdef USE_SSE2
    __m128 s = _mm_load1_ps(&scalar);
    __m128 v1 = _mm_load_ps(vec + 0);
    __m128 v2 = _mm_load_ps(vec + 4);
    __m128 v3 = _mm_load_ps(vec + 8);
    __m128 v4 = _mm_load_ps(vec + 12);
    t[0] = _mm_add_ps(t[0], _mm_mul_ps(v1, s));
    t[1] = _mm_add_ps(t[1], _mm_mul_ps(v2, s));
    t[2] = _mm_add_ps(t[2], _mm_mul_ps(v3, s));
    t[3] = _mm_add_ps(t[3], _mm_mul_ps(v4, s));    
#else    
    for (int i=0; i<4*4; i++)
    {
        t[i] += (vec[i] * scalar);
    }
#endif 
}


void NeuronSW::m_store1(m_vec_base_t * t, float * layer)
{
#ifdef USE_SSE2
    _mm_store_ps(layer, t[0]);
#else    
    memcpy(layer, t, 4*1*sizeof(float));
#endif 
}


void NeuronSW::m_store2(m_vec_base_t * t, float * layer)
{
#ifdef USE_SSE2
    _mm_store_ps(layer + 0, t[0]);
    _mm_store_ps(layer + 4, t[1]);
#else    
    memcpy(layer, t, 4*2*sizeof(float));
#endif 
}


void NeuronSW::m_store3(m_vec_base_t * t, float * layer)
{
#ifdef USE_SSE2
    _mm_store_ps(layer + 0, t[0]);
    _mm_store_ps(layer + 4, t[1]);
    _mm_store_ps(layer + 8, t[2]);
#else    
    memcpy(layer, t, 4*3*sizeof(float));
#endif 
}


void NeuronSW::m_store4(m_vec_base_t * t, float * layer)
{
#ifdef USE_SSE2
    _mm_store_ps(layer + 0, t[0]);
    _mm_store_ps(layer + 4, t[1]);
    _mm_store_ps(layer + 8, t[2]);
    _mm_store_ps(layer + 12, t[3]);
#else    
    memcpy(layer, t, 4*4*sizeof(float));
#endif 
}


void NeuronSW::m_add1_mxXvec2(m_vec_base_t * t, float * mx, float * vec)
{
#ifdef USE_SSE2
    for (int i=0; i<2; i++)
    {
        __m128 v4 = _mm_load_ps(vec);
        for (int m=0; m<1; m++)
        {
            __m128 u1 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(0,0,0,0));
            __m128 u2 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(1,1,1,1));
            __m128 u3 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(2,2,2,2));
            __m128 u4 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(3,3,3,3));
            __m128 prod1 = _mm_mul_ps(u1, _mm_load_ps(mx));
            mx+=4;
            __m128 prod2 = _mm_mul_ps(u2, _mm_load_ps(mx));
            mx+=4;
            __m128 prod3 = _mm_mul_ps(u3, _mm_load_ps(mx));
            mx+=4;
            __m128 prod4 = _mm_mul_ps(u4, _mm_load_ps(mx));            
            mx+=4;
            t[m] = _mm_add_ps(t[m],_mm_add_ps(_mm_add_ps(prod1, prod2), _mm_add_ps(prod3, prod4)));
        }
        vec += 4;
    }      
#else    
    for (int i=0; i<2; i++)
    {
        for (int m=0; m<1; m++)
        {
            for (int j=0; j<4; j++)
            {
                float sum = 0;
                for (int k=0; k<4; k++)
                {
                    sum += *mx * vec[k];
                    mx++;
                }
                t[m*4+j] += sum;
            }
        }
        vec += 4;
    }
#endif 
}


void NeuronSW::m_add1_mxXvec4(m_vec_base_t * t, float * mx, float * vec)
{
#ifdef USE_SSE2
    for (int i=0; i<4; i++)
    {
        __m128 v4 = _mm_load_ps(vec);
        for (int m=0; m<1; m++)
        {
            __m128 u1 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(0,0,0,0));
            __m128 u2 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(1,1,1,1));
            __m128 u3 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(2,2,2,2));
            __m128 u4 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(3,3,3,3));
            __m128 prod1 = _mm_mul_ps(u1, _mm_load_ps(mx));
            mx+=4;
            __m128 prod2 = _mm_mul_ps(u2, _mm_load_ps(mx));
            mx+=4;
            __m128 prod3 = _mm_mul_ps(u3, _mm_load_ps(mx));
            mx+=4;
            __m128 prod4 = _mm_mul_ps(u4, _mm_load_ps(mx));            
            mx+=4;
            t[m] = _mm_add_ps(t[m],_mm_add_ps(_mm_add_ps(prod1, prod2), _mm_add_ps(prod3, prod4)));
        }
        vec += 4;
    }      
#else    
    for (int i=0; i<4; i++)
    {
        for (int m=0; m<1; m++)
        {
            for (int j=0; j<4; j++)
            {
                float sum = 0;
                for (int k=0; k<4; k++)
                {
                    sum += *mx * vec[k];
                    mx++;
                }
                t[m*4+j] += sum;
            }
        }
        vec += 4;
    }
#endif 
}


void NeuronSW::m_add2_mxXvec2(m_vec_base_t * t, float * mx, float * vec)
{
#ifdef USE_SSE2
    for (int i=0; i<2; i++)
    {
        __m128 v4 = _mm_load_ps(vec);
        for (int m=0; m<2; m++)
        {
            __m128 u1 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(0,0,0,0));
            __m128 u2 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(1,1,1,1));
            __m128 u3 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(2,2,2,2));
            __m128 u4 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(3,3,3,3));
            __m128 prod1 = _mm_mul_ps(u1, _mm_load_ps(mx));
            mx+=4;
            __m128 prod2 = _mm_mul_ps(u2, _mm_load_ps(mx));
            mx+=4;
            __m128 prod3 = _mm_mul_ps(u3, _mm_load_ps(mx));
            mx+=4;
            __m128 prod4 = _mm_mul_ps(u4, _mm_load_ps(mx));            
            mx+=4;
            t[m] = _mm_add_ps(t[m],_mm_add_ps(_mm_add_ps(prod1, prod2), _mm_add_ps(prod3, prod4)));
        }
        vec += 4;
    }      
#else    
    for (int i=0; i<2; i++)
    {
        for (int m=0; m<2; m++)
        {
            for (int j=0; j<4; j++)
            {
                float sum = 0;
                for (int k=0; k<4; k++)
                {
                    sum += *mx * vec[k];
                    mx++;
                }
                t[m*4+j] += sum;
            }
        }
        vec += 4;
    }
#endif 
}


void NeuronSW::m_add3_mxXvec4(m_vec_base_t * t, float * mx, float * vec)
{
#ifdef USE_SSE2
    for (int i=0; i<4; i++)
    {
        __m128 v4 = _mm_load_ps(vec);
        for (int m=0; m<3; m++)
        {
            __m128 u1 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(0,0,0,0));
            __m128 u2 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(1,1,1,1));
            __m128 u3 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(2,2,2,2));
            __m128 u4 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(3,3,3,3));
            __m128 prod1 = _mm_mul_ps(u1, _mm_load_ps(mx));
            mx+=4;
            __m128 prod2 = _mm_mul_ps(u2, _mm_load_ps(mx));
            mx+=4;
            __m128 prod3 = _mm_mul_ps(u3, _mm_load_ps(mx));
            mx+=4;
            __m128 prod4 = _mm_mul_ps(u4, _mm_load_ps(mx));            
            mx+=4;
            t[m] = _mm_add_ps(t[m],_mm_add_ps(_mm_add_ps(prod1, prod2), _mm_add_ps(prod3, prod4)));
        }
        vec += 4;
    }      
#else    
    for (int i=0; i<4; i++)
    {
        for (int m=0; m<3; m++)
        {
            for (int j=0; j<4; j++)
            {
                float sum = 0;
                for (int k=0; k<4; k++)
                {
                    sum += *mx * vec[k];
                    mx++;
                }
                t[m*4+j] += sum;
            }
        }
        vec += 4;
    }
#endif 
}


void NeuronSW::m_add4_mxXvec4(m_vec_base_t * t, float * mx, float * vec)
{
#ifdef USE_SSE2
    for (int i=0; i<4; i++)
    {
        __m128 v4 = _mm_load_ps(vec);
        for (int m=0; m<4; m++)
        {
            __m128 u1 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(0,0,0,0));
            __m128 u2 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(1,1,1,1));
            __m128 u3 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(2,2,2,2));
            __m128 u4 = _mm_shuffle_ps(v4,v4, _MM_SHUFFLE(3,3,3,3));
            __m128 prod1 = _mm_mul_ps(u1, _mm_load_ps(mx));
            mx+=4;
            __m128 prod2 = _mm_mul_ps(u2, _mm_load_ps(mx));
            mx+=4;
            __m128 prod3 = _mm_mul_ps(u3, _mm_load_ps(mx));
            mx+=4;
            __m128 prod4 = _mm_mul_ps(u4, _mm_load_ps(mx));            
            mx+=4;
            t[m] = _mm_add_ps(t[m],_mm_add_ps(_mm_add_ps(prod1, prod2), _mm_add_ps(prod3, prod4)));
        }
        vec += 4;
    }      
#else    
    for (int i=0; i<4; i++)
    {
        for (int m=0; m<4; m++)
        {
            for (int j=0; j<4; j++)
            {
                float sum = 0;
                for (int k=0; k<4; k++)
                {
                    sum += *mx * vec[k];
                    mx++;
                }
                t[m*4+j] += sum;
            }
        }
        vec += 4;
    }
#endif 
}


void NeuronSW::m_alpha2(m_vec_base_t * t, float * alpha)
{
#ifdef USE_SSE2
    __m128 zero = _mm_setzero_ps();
    __m128 a1 = _mm_load_ps(alpha + 0);
    __m128 a2 = _mm_load_ps(alpha + 4);
    t[0] = _mm_add_ps(_mm_max_ps(t[0], zero), _mm_mul_ps(_mm_min_ps(t[0], zero), a1));
    t[1] = _mm_add_ps(_mm_max_ps(t[1], zero), _mm_mul_ps(_mm_min_ps(t[1], zero), a2));
#else    
    for (int i=0; i<4*2; i++)
    {
        t[i] = ((t[i] < 0) ? 0.0 : t[i]) + alpha[i]*((t[i] > 0) ? 0.0 : t[i]);
    }
#endif 
}


void NeuronSW::m_alpha4(m_vec_base_t * t, float * alpha)
{
#ifdef USE_SSE2
    __m128 zero = _mm_setzero_ps();
    __m128 a1 = _mm_load_ps(alpha + 0);
    __m128 a2 = _mm_load_ps(alpha + 4);
    __m128 a3 = _mm_load_ps(alpha + 8);
    __m128 a4 = _mm_load_ps(alpha + 12);
    t[0] = _mm_add_ps(_mm_max_ps(t[0], zero), _mm_mul_ps(_mm_min_ps(t[0], zero), a1));
    t[1] = _mm_add_ps(_mm_max_ps(t[1], zero), _mm_mul_ps(_mm_min_ps(t[1], zero), a2));
    t[2] = _mm_add_ps(_mm_max_ps(t[2], zero), _mm_mul_ps(_mm_min_ps(t[2], zero), a3));
    t[3] = _mm_add_ps(_mm_max_ps(t[3], zero), _mm_mul_ps(_mm_min_ps(t[3], zero), a4));
#else    
    for (int i=0; i<4*4; i++)
    {
        t[i] = ((t[i] < 0) ? 0.0 : t[i]) + alpha[i]*((t[i] > 0) ? 0.0 : t[i]);
    }
#endif 
}


void NeuronSW::m_integerize1(m_vec_base_t * t, uint8_t * plane, unsigned int stride)
{
#ifdef USE_SSE2
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
    plane[0+0] = pix[0];
    plane[0+1] = pix[2];
    plane[stride+0] = pix[1];
    plane[stride+1] = pix[3];
#else    
    uint8_t pix[4];
    for (int i=0; i<4*1; i++)
    {
        t[i] *= 255.0;
        t[i] = std::round(t[i]);
        if (t[i] < 0) t[i] = 0;
        if (t[i] > 255) t[i] = 255;
        pix[i] = t[i];
    }
    plane[0+0] = pix[0];
    plane[0+1] = pix[2];
    plane[stride+0] = pix[1];
    plane[stride+1] = pix[3];
#endif 
}


void NeuronSW::m_integerize3(m_vec_base_t * t, uint8_t * plane, unsigned int stride)
{
#ifdef USE_SSE2
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
    plane[0+0] = pix[0];
    plane[stride+0] = pix[1];
    plane[2*stride+0] = pix[2];
    intvalue = _mm_cvtps_epi32(t[1]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0+1] = pix[0];
    plane[stride+1] = pix[1];
    plane[2*stride+1] = pix[2];
    intvalue = _mm_cvtps_epi32(t[2]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0+2] = pix[0];
    plane[stride+2] = pix[1];
    plane[2*stride+2] = pix[2];
#else    
    for (int i=0; i<4*3; i++)
    {
        t[i] *= 255.0;
        t[i] = std::round(t[i]);
        if (t[i] < 0) t[i] = 0;
        if (t[i] > 255) t[i] = 255;
    }
    for (int i=0; i<3; i++)
    {
        for (int j=0; j<3; j++)
        {
            plane[j*stride + i] = t[i*4+j];
        }
    }
#endif 
}


void NeuronSW::m_integerize4(m_vec_base_t * t, uint8_t * plane, unsigned int stride)
{
#ifdef USE_SSE2
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
    plane[0*stride+0] = pix[0];
    plane[1*stride+0] = pix[1];
    plane[2*stride+0] = pix[2];
    plane[3*stride+0] = pix[3];
    intvalue = _mm_cvtps_epi32(t[1]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0*stride+1] = pix[0];
    plane[1*stride+1] = pix[1];
    plane[2*stride+1] = pix[2];
    plane[3*stride+1] = pix[3];
    intvalue = _mm_cvtps_epi32(t[2]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0*stride+2] = pix[0];
    plane[1*stride+2] = pix[1];
    plane[2*stride+2] = pix[2];
    plane[3*stride+2] = pix[3];
    intvalue = _mm_cvtps_epi32(t[3]);
    intvalue = _mm_packs_epi32(intvalue,intvalue);
    intvalue = _mm_packus_epi16(intvalue,intvalue);
    pixels = _mm_cvtsi128_si32(intvalue);
    plane[0*stride+3] = pix[0];
    plane[1*stride+3] = pix[1];
    plane[2*stride+3] = pix[2];
    plane[3*stride+3] = pix[3];
#else    
    for (int i=0; i<4*4; i++)
    {
        t[i] *= 255.0;
        t[i] = std::round(t[i]);
        if (t[i] < 0) t[i] = 0;
        if (t[i] > 255) t[i] = 255;
    }
    for (int i=0; i<4; i++)
    {
        for (int j=0; j<4; j++)
        {
            plane[j*stride + i] = t[i*4+j];
        }
    }
#endif 
}

