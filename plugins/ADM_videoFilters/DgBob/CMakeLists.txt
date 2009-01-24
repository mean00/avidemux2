INCLUDE(vf_plugin)


SET(ADM_vf_blendDgBob_SRCS ADM_vidDGbob.cpp)

ADD_LIBRARY(ADM_vf_blendDgBob SHARED ${ADM_vf_blendDgBob_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_blendDgBob)
INSTALL_VIDEOFILTER(ADM_vf_blendDgBob)
