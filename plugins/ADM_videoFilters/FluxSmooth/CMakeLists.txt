INCLUDE(vf_plugin)


SET(ADM_vf_FluxSmooth_SRCS ADM_vidFlux.cpp)

ADD_LIBRARY(ADM_vf_FluxSmooth SHARED ${ADM_vf_FluxSmooth_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_FluxSmooth)
INSTALL_VIDEOFILTER(ADM_vf_FluxSmooth)
