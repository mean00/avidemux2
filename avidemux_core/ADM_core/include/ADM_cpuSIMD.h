#ifndef ADM_CPUSIMD_H
#define ADM_CPUSIMD_H

#include "ADM_coreConfig.h"

#undef ADM_CPU_HAS_SIMD
#undef ADM_CPU_HAS_SIMD_AVX

// SSE1 and SSE2 are mandatory parts of x86_64
#if defined(ADM_CPU_X86_64)
#  include <immintrin.h>
#  define ADM_CPU_HAS_SIMD
#  define ADM_CPU_HAS_SIMD_AVX

  // functions with targets above SSE2 require runtime CpuCap check before use!

#  if defined(__GNUC__) && !defined(__INTEL_COMPILER)
    // GCC and CLANG target attributes
#    define ADM_CPU_SIMD_TARGET_SSE3    __attribute__((target("sse3")))
#    define ADM_CPU_SIMD_TARGET_SSSE3   __attribute__((target("ssse3")))
#    define ADM_CPU_SIMD_TARGET_SSE4    __attribute__((target("sse4.1")))
#    define ADM_CPU_SIMD_TARGET_SSE42   __attribute__((target("sse4")))     // "sse4" == both 4.1 & 4.2
#    define ADM_CPU_SIMD_TARGET_AVX     __attribute__((target("avx")))
#    define ADM_CPU_SIMD_TARGET_AVX2    __attribute__((target("avx2")))
#  else
    // MSVC and ICC do not require extension enablement
#    define ADM_CPU_SIMD_TARGET_SSE3
#    define ADM_CPU_SIMD_TARGET_SSSE3
#    define ADM_CPU_SIMD_TARGET_SSE4
#    define ADM_CPU_SIMD_TARGET_SSE42
#    define ADM_CPU_SIMD_TARGET_AVX
#    define ADM_CPU_SIMD_TARGET_AVX2
#  endif
#elif defined(ADM_CPU_ARM64)
#  warning Using sse2neon SIMD translator for ARMv8-A 64-bit target
#  include "sse2neon.h"
#  define ADM_CPU_HAS_SIMD
#  define ADM_CPU_SIMD_TARGET_SSE3
#  define ADM_CPU_SIMD_TARGET_SSSE3
#  define ADM_CPU_SIMD_TARGET_SSE4
#  define ADM_CPU_SIMD_TARGET_SSE42
#  define ADM_CPU_SIMD_TARGET_AVX       static_assert(false, "AVX not supported by sse2neon!");
#  define ADM_CPU_SIMD_TARGET_AVX2      static_assert(false, "AVX2 not supported by sse2neon!");
#endif

#endif