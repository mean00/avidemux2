INCLUDE(vf_plugin)


SET(ADM_vf_vflip_SRCS verticalFlip.cpp)

ADD_LIBRARY(ADM_vf_vflip SHARED ${ADM_vf_vflip_SRCS})

INIT_VIDEO_FILTER(ADM_vf_vflip)
INSTALL_VIDEO_FILTER(ADM_vf_vflip)
