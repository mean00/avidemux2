INCLUDE(vf_plugin)


SET(ADM_vf_decimate_SRCS ADM_vidDecDec.cpp  videoFunctions.cpp)

ADD_LIBRARY(ADM_vf_decimate SHARED ${ADM_vf_decimate_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_decimate)
INSTALL_VIDEOFILTER(ADM_vf_decimate)

IF (ADM_CPU_X86)
    ADD_SOURCE_CFLAGS(ADM_vidDecDec.cpp "-DUSE_SSE")
ENDIF (ADM_CPU_X86)