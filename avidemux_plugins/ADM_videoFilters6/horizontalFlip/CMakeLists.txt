INCLUDE(vf_plugin)


SET(ADM_vf_hflip_SRCS horizontalFlip.cpp)

ADD_VIDEO_FILTER(ADM_vf_hflip ${ADM_vf_hflip_SRCS})

INIT_VIDEO_FILTER(ADM_vf_hflip)
INSTALL_VIDEO_FILTER(ADM_vf_hflip)
