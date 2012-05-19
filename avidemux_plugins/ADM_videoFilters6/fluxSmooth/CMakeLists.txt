INCLUDE(vf_plugin)


SET(ADM_vf_FluxSmooth_SRCS ADM_vidFlux.cpp)

ADD_VIDEO_FILTER(ADM_vf_FluxSmooth ${ADM_vf_FluxSmooth_SRCS})

INIT_VIDEO_FILTER(ADM_vf_FluxSmooth)
INSTALL_VIDEO_FILTER(ADM_vf_FluxSmooth)
