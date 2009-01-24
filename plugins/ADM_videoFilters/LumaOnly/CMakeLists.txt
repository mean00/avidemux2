INCLUDE(vf_plugin)


SET(ADM_vf_lumaonly_SRCS ADM_vidLuma.cpp)

ADD_LIBRARY(ADM_vf_lumaonly SHARED ${ADM_vf_lumaonly_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_lumaonly)
INSTALL_VIDEOFILTER(ADM_vf_lumaonly)
