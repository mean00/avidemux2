INCLUDE(vf_plugin)


SET(ADM_vf_swapuv_SRCS ADM_vidUVSwap.cpp)

ADD_LIBRARY(ADM_vf_swapuv SHARED ${ADM_vf_swapuv_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_swapuv)
INSTALL_VIDEOFILTER(ADM_vf_swapuv)
