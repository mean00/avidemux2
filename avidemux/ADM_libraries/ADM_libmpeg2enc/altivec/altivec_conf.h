/* altivec_conf.h, this file is part of the
 * AltiVec optimized library for MJPEG tools MPEG-1/2 Video Encoder
 * Copyright (C) 2002  James Klicman <james@klicman.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * development settings: VERIFY and BENCHMARK are mutually exclusive
 */

#undef ALTIVEC_VERIFY
#undef ALTIVEC_BENCHMARK
#undef ALTIVEC_AMBER

#define ALTIVEC_DST /* use data stream touch */

#if defined(ALTIVEC_VERIFY)
/* {{{ */
/*
 * define each function to verify with it's comparable function
 */
#define ALTIVEC_TEST_build_sub44_mests_WITH         build_sub44_mests
#define ALTIVEC_TEST_build_sub22_mests_WITH         build_sub22_mests 
#define ALTIVEC_TEST_find_best_one_pel_WITH         find_best_one_pel 
#define ALTIVEC_TEST_sub_mean_reduction_WITH        sub_mean_reduction 
#define ALTIVEC_TEST_sad_00_WITH                    sad_00 
#define ALTIVEC_TEST_sad_01_WITH                    sad_01 
#define ALTIVEC_TEST_sad_10_WITH                    sad_10 
#define ALTIVEC_TEST_sad_11_WITH                    sad_11 
#define ALTIVEC_TEST_bsad_WITH                      bsad
#define ALTIVEC_TEST_sumsq_WITH                     sumsq 
#define ALTIVEC_TEST_bsumsq_WITH                    bsumsq 
#define ALTIVEC_TEST_sumsq_sub22_WITH               sumsq_sub22 
#define ALTIVEC_TEST_bsumsq_sub22_WITH              bsumsq_sub22 
#define ALTIVEC_TEST_subsample_image_WITH           subsample_image 
#define ALTIVEC_TEST_variance_WITH                  variance 
#define ALTIVEC_TEST_quant_non_intra_WITH           quant_non_intra 
#define ALTIVEC_TEST_quant_weight_coeff_intra_WITH  quant_weight_coeff_intra 
#define ALTIVEC_TEST_quant_weight_coeff_inter_WITH  quant_weight_coeff_inter 
#define ALTIVEC_TEST_iquant_non_intra_m1_WITH       iquant_non_intra_m1
#define ALTIVEC_TEST_iquant_non_intra_m2_WITH       iquant_non_intra_m2
#define ALTIVEC_TEST_iquant_intra_m1_WITH           iquant_intra_m1
#define ALTIVEC_TEST_iquant_intra_m2_WITH           iquant_intra_m2
#define ALTIVEC_TEST_add_pred_WITH                  add_pred 
#define ALTIVEC_TEST_sub_pred_WITH                  sub_pred 
#define ALTIVEC_TEST_pred_comp_WITH                 pred_comp 
#define ALTIVEC_TEST_field_dct_best_WITH            field_dct_best
#define ALTIVEC_TEST_fdct_WITH                      /* output range test */    
#define ALTIVEC_TEST_idct_WITH                      /* output range test */    

#  include "verify.h"
#  define ALTIVEC_TEST_SUFFIX(name) name##_altivec_verify
#  define ALTIVEC_FUNCTION(name,ret,def)                                     \
    ret name##_altivec def;                                                  \
    ret name##_altivec_verify def;
#  define ALTIVEC_TEST ALTIVEC_TEST_VERIFY

/* }}} */
#elif defined(ALTIVEC_BENCHMARK)
/* {{{ */
/* define each function to benchmark with it's comparable function */
#undef  ALTIVEC_TEST_sad_00_WITH                    sad_00 
#undef  ALTIVEC_TEST_sad_01_WITH                    sad_01
#undef  ALTIVEC_TEST_sad_10_WITH                    sad_10 
#undef  ALTIVEC_TEST_sad_11_WITH                    sad_11
#undef  ALTIVEC_TEST_bsad_WITH                      bsad
#undef  ALTIVEC_TEST_sumsq_WITH                     sumsq
#undef  ALTIVEC_TEST_bsumsq_WITH                    bsumsq 
#undef  ALTIVEC_TEST_sumsq_sub22_WITH               sumsq_sub22
#undef  ALTIVEC_TEST_bsumsq_sub22_WITH              bsumsq_sub22 
#undef  ALTIVEC_TEST_quant_non_intra_WITH           quant_non_intra 
#undef  ALTIVEC_TEST_quant_weight_coeff_intra_WITH  quant_weight_coeff_intra 
#undef  ALTIVEC_TEST_quant_weight_coeff_inter_WITH  quant_weight_coeff_inter 
#undef  ALTIVEC_TEST_variance_WITH                  variance 
#undef  ALTIVEC_TEST_sub_pred_WITH                  sub_pred 
#undef  ALTIVEC_TEST_add_pred_WITH                  add_pred 
#undef  ALTIVEC_TEST_pred_comp_WITH                 pred_comp 
#undef  ALTIVEC_TEST_subsample_image_WITH           subsample_image 
#undef  ALTIVEC_TEST_field_dct_best_WITH            field_dct_best
/* the following functions may call other functions and should be */
/* benchmarked separately. */
#undef  ALTIVEC_TEST_find_best_one_pel_WITH         find_best_one_pel 
#undef  ALTIVEC_TEST_build_sub44_mests_WITH         build_sub44_mests
#undef  ALTIVEC_TEST_build_sub22_mests_WITH         build_sub22_mests
/* can't benchmark the following functions since they modify their input */
#undef  ALTIVEC_TEST_sub_mean_reduction_WITH        sub_mean_reduction 
/* the following functions modify their input but it shouldn't affect timing */
#undef  ALTIVEC_TEST_iquant_non_intra_m1_WITH       iquant_non_intra_m1
#undef  ALTIVEC_TEST_iquant_non_intra_m2_WITH       iquant_non_intra_m2
#undef  ALTIVEC_TEST_iquant_intra_m1_WITH           iquant_intra_m1
#undef  ALTIVEC_TEST_iquant_intra_m2_WITH           iquant_intra_m2
#undef  ALTIVEC_TEST_fdct_WITH                      fdct
#undef  ALTIVEC_TEST_idct_WITH                      idct

/* turn off (undef) DST during benchmarking, it only slows the function down
 * since everything will be cached due to the benchmark loop.
 */
#undef ALTIVEC_DST

/* default benchmark settings. these values can be redefined before
 * calling ALTIVEC_TEST() to customize values on a per function basis.
 */
#define BENCHMARK_FREQUENCY  2000   /* benchmark every (n) calls */
#define BENCHMARK_MAX_RUNS   20     /* benchmark only (n) times */
#define BENCHMARK_ITERATIONS 100000 /* starting point for calibration */
#define BENCHMARK_INCREMENT  1.5    /* multiply iterations by this amount */
#define BENCHMARK_PRECISION  0.005  /* calibration goal */
#define BENCHMARK_PASSES     2      /* number of times the calibration goal
                                     * must be met before continuting
                                     */
#define BENCHMARK_TIMELIMIT  4      /* time limit in seconds for benchmark
                                     * if calibration goal can't be met.
                                     */
#define BENCHMARK_PROLOG            /* code to execute before benchmark */
#define BENCHMARK_EPILOG            /* code to execute after benchmark */

#  include "benchmark.h"
#  define ALTIVEC_TEST_SUFFIX(name) name##_altivec_benchmark
#  define ALTIVEC_FUNCTION(name,ret,def)                                     \
    ret name##_altivec def;                                                  \
    ret name##_altivec_benchmark def;
#  define ALTIVEC_TEST ALTIVEC_TEST_BENCHMARK

/* }}} */
#elif defined(ALTIVEC_AMBER)
/* {{{ */
/* define each function to amber */
/* since some functions modify their input all are tested individually */
#if 1 /* altivec = 1, orignal C = 0 */
/* amber optimized functions */
#if 0 /* group1 = 1, group2 = 0 */
#define ALTIVEC_TEST_sad_00_WITH                 sad_00_altivec
#define ALTIVEC_TEST_sad_01_WITH                 sad_01_altivec
#define ALTIVEC_TEST_sad_10_WITH                 sad_10_altivec
#define ALTIVEC_TEST_sad_11_WITH                 sad_11_altivec
#undef  ALTIVEC_TEST_bsad_WITH                   bsad_altivec
#define ALTIVEC_TEST_sumsq_WITH                  sumsq_altivec
#define ALTIVEC_TEST_bsumsq_WITH                 bsumsq_altivec
#define ALTIVEC_TEST_sumsq_sub22_WITH            sumsq_sub22_altivec
#define ALTIVEC_TEST_bsumsq_sub22_WITH           bsumsq_sub22_altivec
#define ALTIVEC_TEST_quant_non_intra_WITH        quant_non_intra_altivec
#define ALTIVEC_TEST_quant_weight_coeff_intra_WITH quant_weight_coeff_intra_altivec
#define ALTIVEC_TEST_quant_weight_coeff_inter_WITH quant_weight_coeff_inter_altivec
#define ALTIVEC_TEST_iquant_non_intra_m1_WITH    iquant_non_intra_m1_altivec
#define ALTIVEC_TEST_iquant_non_intra_m2_WITH    iquant_non_intra_m2_altivec
#define ALTIVEC_TEST_iquant_intra_m1_WITH        iquant_intra_m1_altivec
#define ALTIVEC_TEST_iquant_intra_m2_WITH        iquant_intra_m2_altivec
#undef  ALTIVEC_TEST_sub_mean_reduction_WITH     sub_mean_reduction_altivec
#define ALTIVEC_TEST_variance_WITH               variance_altivec
#define ALTIVEC_TEST_sub_pred_WITH               sub_pred_altivec
#define ALTIVEC_TEST_add_pred_WITH               add_pred_altivec
#define ALTIVEC_TEST_pred_comp_WITH              pred_comp_altivec
#define ALTIVEC_TEST_subsample_image_WITH        subsample_image_altivec
#define ALTIVEC_TEST_field_dct_best_WITH         field_dct_best_altivec
#define ALTIVEC_TEST_fdct_WITH                   fdct_altivec
#define ALTIVEC_TEST_idct_WITH                   idct_altivec
#else /* the following call other amber functions, must amber separately */
#define ALTIVEC_TEST_build_sub44_mests_WITH      build_sub44_mests_altivec
#define ALTIVEC_TEST_build_sub22_mests_WITH      build_sub22_mests_altivec
#define ALTIVEC_TEST_find_best_one_pel_WITH      find_best_one_pel_altivec
#endif
#else
/* amber original functions */
#if 0 /* group1 = 1, group2 = 0 */
#define ALTIVEC_TEST_sad_00_WITH                 sad_00 
#define ALTIVEC_TEST_sad_01_WITH                 sad_01
#define ALTIVEC_TEST_sad_10_WITH                 sad_10 
#define ALTIVEC_TEST_sad_11_WITH                 sad_11
#undef  ALTIVEC_TEST_bsad_WITH                   bsad
#define ALTIVEC_TEST_sumsq_WITH                  sumsq
#define ALTIVEC_TEST_bsumsq_WITH                 bsumsq 
#define ALTIVEC_TEST_sumsq_sub22_WITH            sumsq_sub22
#define ALTIVEC_TEST_bsumsq_sub22_WITH           bsumsq_sub22 
#define ALTIVEC_TEST_quant_non_intra_WITH        quant_non_intra 
#define ALTIVEC_TEST_quant_weight_coeff_intra_WITH quant_weight_coeff_intra
#define ALTIVEC_TEST_quant_weight_coeff_inter_WITH quant_weight_coeff_inter
#define ALTIVEC_TEST_iquant_non_intra_m1_WITH    iquant_non_intra_m1
#define ALTIVEC_TEST_iquant_non_intra_m2_WITH    iquant_non_intra_m2
#define ALTIVEC_TEST_iquant_intra_m1_WITH        iquant_intra_m1
#define ALTIVEC_TEST_iquant_intra_m2_WITH        iquant_intra_m2
#undef  ALTIVEC_TEST_sub_mean_reduction_WITH     sub_mean_reduction 
#define ALTIVEC_TEST_variance_WITH               variance 
#define ALTIVEC_TEST_sub_pred_WITH               sub_pred 
#define ALTIVEC_TEST_add_pred_WITH               add_pred 
#define ALTIVEC_TEST_pred_comp_WITH              pred_comp 
#define ALTIVEC_TEST_subsample_image_WITH        subsample_image 
#define ALTIVEC_TEST_field_dct_best_WITH         field_dct_best
#define ALTIVEC_TEST_fdct_WITH                   fdct
#define ALTIVEC_TEST_idct_WITH                   idct
#else /* the following call other amber functions, must amber separately */
#define ALTIVEC_TEST_build_sub44_mests_WITH      build_sub44_mests
#define ALTIVEC_TEST_build_sub22_mests_WITH      build_sub22_mests
#define ALTIVEC_TEST_find_best_one_pel_WITH      find_best_one_pel 
#endif
#endif

#define AMBER_MAX_TRACES 50  /* number of times to trace each function */

#if 0 /* old global amber trace activation */
#define AMBER_ENABLE
#define AMBER_MAX_TRACES 1 /* trace each function once */
#define AMBER_MAX_EXIT 0
#endif

#  define ALTIVEC_TEST_SUFFIX(name) name##_altivec_amber
#  define ALTIVEC_FUNCTION(name,ret,def)                                     \
    ret name##_altivec def;                                                  \
    ret name##_altivec_amber def;
#  define ALTIVEC_TEST ALTIVEC_TEST_AMBER
/* }}} */
#else /* PRODUCTION */
#  define ALTIVEC_FUNCTION(name,ret,def)                                     \
    ret name##_altivec def;
#endif

/* non-configurable macro definitions {{{ */

#define ALTIVEC_SUFFIX(name) name##_altivec

/* macros to assist in code generation */
#define AVCAT(a,b)   _AVCAT(a,b) /* allow expansion */
#define _AVCAT(a,b)  a##b        /* concatenate */
#define AVSTR(a)     _AVSTR(a)   /* allow expansion */
#define _AVSTR(a)    #a          /* convert to string */
/* AVRET* expand differently depending on return type */
#define AVRETDECL(type,name)  _AVRETDECL(_AVRETDECL_##type,name)
#define _AVRETDECL(type,name)  type(name)
#define _AVRETDECL_int(name)   int name
#define _AVRETDECL_void(name)  /* void name */
#define AVRETSET(type,var,call)  _AVRETSET(_AVRETSET_##type,var,call)
#define _AVRETSET(type,var,call)  type(var,call)
#define _AVRETSET_int(var,call)      var = call
#define _AVRETSET_void(var,call)  /* var = */ call
#define AVRETURN(type,var)  _AVRETURN(_AVRETURN_##type,var)
#define _AVRETURN(type,var)  type(var)
#define _AVRETURN_int(var)   return var
#define _AVRETURN_void(var)  /* return var */
/* printf format codes used by ALTIVEC_VERIFY */
#define AVFMT(ret)  _AVFMT(_AVFMT_##ret)
#define _AVFMT(ret) ret
#define _AVFMT_int    "%d"
#define _AVFMT_float  "%f"


#define ALTIVEC_TEST_FUNCTION(name) defined(ALTIVEC_TEST_##name##_WITH)

#define ALTIVEC_TEST_WITH(name) \
        _ALTIVEC_TEST_WITH(ALTIVEC_TEST_##name##_WITH)
#define _ALTIVEC_TEST_WITH(name) name /* allow expansion */

/* }}} */

/* vim:set sw=4 softtabstop=4 foldmethod=marker foldlevel=0: */
