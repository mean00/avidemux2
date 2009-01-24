INCLUDE(vf_plugin)


SET(ADM_vf_reverse_SRCS ADM_vidReverse.cpp)

ADD_LIBRARY(ADM_vf_reverse SHARED ${ADM_vf_reverse_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_reverse)
INSTALL_VIDEOFILTER(ADM_vf_reverse)
