INCLUDE(vf_plugin)


SET(ADM_vf_mSmooth_SRCS ADM_vidMsmooth.cpp)

ADD_LIBRARY(ADM_vf_mSmooth SHARED ${ADM_vf_mSmooth_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_mSmooth)
INSTALL_VIDEOFILTER(ADM_vf_mSmooth)
