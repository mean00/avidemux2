INCLUDE(vf_plugin)


SET(ADM_vf_vflip_SRCS verticalFlip.cpp)

ADD_VIDEO_FILTER(ADM_vf_vflip ${ADM_vf_vflip_SRCS})

INIT_VIDEO_FILTER(ADM_vf_vflip)
INSTALL_VIDEO_FILTER(ADM_vf_vflip)
