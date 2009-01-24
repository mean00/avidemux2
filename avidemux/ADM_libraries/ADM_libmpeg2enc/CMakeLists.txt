SET(ADM_LIB ADM_libmpeg2enc)

SET(${ADM_LIB}_SRCS 
ad_layer.cc           fdct_x86.cc           mblock_bsad_mmx.cc          motionest.cc     predcomp_mmxe.cc  putpic.cc        readpic.cc       yuv4mpeg.cc
build_sub22_mests.cc  feedframe.cc          mblock_bsumsq_mmx.cc        motionsearch.cc  predict.cc        putseq.cc        stats.cc         yuv4mpeg_ratio.cc
build_sub44_mests.cc  find_best_one_pel.cc  mblock_sad_mmx.cc           mpeg2enc.cc      predict_mmx.cc    putvlc.cc        synchrolib.cc
conform.cc            idct.cc               mblock_sad_mmxe.cc          mpeg2enc_if.cc   predict_ref.cc    quantize.cc      transfrm.cc
cpu_accel.cc          idctdata.cc           mblock_sub44_sads_x86.cc    mpeg2param.cc    predict_x86.cc    quantize_ref.cc  transfrm_ref.cc
fdctdata.cc           idct_mmx.cc           mpeg2setup.cc               putbits.cc        quantize_x86.cc  transfrm_x86.cc
fdct_mmx.cc           ioio.c                mblock_sumsq_mmx.cc         mpegconsts.cc    puthdr.cc         quant_mmx2.cc    writepic.cc
fdctref.cc            macroblock.cc         motion.cc                   predcomp_mmx.cc  putmpg.cc         ratectl.cc       ADM_mpe2enc.cpp
)

IF (ADM_CPU_ALTIVEC)
	SET(${ADM_LIB}_SRCS ${${ADM_LIB}_SRCS}
		altivec/add_pred.c   altivec/amber.c  altivec/benchmark.c  altivec/bsad.c  altivec/bsumsq.c  altivec/bsumsq_sub22.c  altivec/build_sub22_mests.c
		altivec/build_sub44_mests.c  altivec/detect.c  altivec/fdct.c  altivec/fdct_idct.c  altivec/field_dct_best.c  altivec/find_best_one_pel.c
		altivec/idct.c  altivec/iquant_intra.c  altivec/iquant_non_intra.c  altivec/motion.c  altivec/pred_comp.c  altivec/quant_non_intra.c
		altivec/quant_weight_coeff_sum.c  altivec/quantize.c  altivec/sad_00.c  altivec/sad_01.c  altivec/sad_10.c  altivec/sad_11.c
		altivec/sub_mean_reduction.c  altivec/sub_mean_reduction_ppc.S  altivec/sub_pred.c  altivec/subsample_image.c  altivec/sumsq.c
		altivec/sumsq_sub22.c  altivec/variance.c)
ENDIF (ADM_CPU_ALTIVEC)

ADD_ADM_LIB_ALL_TARGETS(${ADM_LIB} ${${ADM_LIB}_SRCS})
ADD_CFLAGS_ALL_TARGETS(${ADM_LIB} "-I${CMAKE_CURRENT_SOURCE_DIR}")

IF (ADM_CPU_X86)
	ADD_CFLAGS_ALL_TARGETS(${ADM_LIB} "-DHAVE_X86CPU")

	IF (ADM_CPU_X86_64)
		ADD_CFLAGS_ALL_TARGETS(${ADM_LIB} "-DHAVE_X86_64_CPU")
	ENDIF (ADM_CPU_X86_64)
ELSEIF (ADM_CPU_ALTIVEC)
	ADD_CFLAGS_ALL_TARGETS(${ADM_LIB} "-DHAVE_ALTIVEC")

	IF (NOT APPLE)
		ADD_CFLAGS_ALL_TARGETS(${ADM_LIB} "-DHAVE_ALTIVEC_H")
	ENDIF (NOT APPLE)
ENDIF (ADM_CPU_X86)
