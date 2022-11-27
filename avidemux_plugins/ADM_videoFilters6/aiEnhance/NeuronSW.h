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

#ifdef ADM_CPU_X86_64
#include <immintrin.h>
#define USE_SSE2
#endif

class NeuronSW
{
protected:
    unsigned int            w,h,threads;

#ifdef USE_SSE2       
    typedef __m128 m_vec_base_t;
  #define DECLARE_M_VEC_T(x)    __m128 x [4]
#else
    typedef float m_vec_base_t;
  #define DECLARE_M_VEC_T(x)    float x [16]
#endif
    

    static void     m_load1_bias(m_vec_base_t * t, float * bias);
    static void     m_load2_bias(m_vec_base_t * t, float * bias);
    static void     m_load3_bias(m_vec_base_t * t, float * bias);
    static void     m_load4_bias(m_vec_base_t * t, float * bias);
    static void     m_add2_vec(m_vec_base_t * t, float * vec);
    static void     m_add4_vec(m_vec_base_t * t, float * vec);
    static void     m_add2_vecXs(m_vec_base_t * t, float * vec, float scalar);
    static void     m_add4_vecXs(m_vec_base_t * t, float * vec, float scalar);
    static void     m_store1(m_vec_base_t * t, float * layer);
    static void     m_store2(m_vec_base_t * t, float * layer);
    static void     m_store3(m_vec_base_t * t, float * layer);
    static void     m_store4(m_vec_base_t * t, float * layer);
    static void     m_add1_mxXvec2(m_vec_base_t * t, float * mx, float * vec);
    static void     m_add1_mxXvec4(m_vec_base_t * t, float * mx, float * vec);
    static void     m_add2_mxXvec2(m_vec_base_t * t, float * mx, float * vec);
    static void     m_add3_mxXvec4(m_vec_base_t * t, float * mx, float * vec);
    static void     m_add4_mxXvec4(m_vec_base_t * t, float * mx, float * vec);
    static void     m_alpha2(m_vec_base_t * t, float * alpha);
    static void     m_alpha4(m_vec_base_t * t, float * alpha);
    static void     m_integerize1(m_vec_base_t * t, uint8_t * plane, unsigned int stride);
    static void     m_integerize3(m_vec_base_t * t, uint8_t * plane, unsigned int stride);
    static void     m_integerize4(m_vec_base_t * t, uint8_t * plane, unsigned int stride);
    
public:
                            NeuronSW(int w, int h);
    virtual                 ~NeuronSW();
    virtual void            upscaleY(ADMImage *srcImg, ADMImage *dstImg) {};
        
};

