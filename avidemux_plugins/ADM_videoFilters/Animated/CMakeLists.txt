INCLUDE(vf_plugin)


SET(ADM_vf_vflip_SRCS ADM_vidFlipV.cpp)

ADD_LIBRARY(ADM_vf_vflip SHARED ${ADM_vf_vflip_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_vflip)
INSTALL_VIDEOFILTER(ADM_vf_vflip)
