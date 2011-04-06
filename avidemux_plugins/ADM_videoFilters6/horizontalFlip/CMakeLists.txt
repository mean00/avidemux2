INCLUDE(vf_plugin)


SET(ADM_hf_hflip_SRCS horizontalFlip.cpp)

ADD_VIDEO_FILTER(ADM_hf_hflip ${ADM_hf_hflip_SRCS})

INIT_VIDEO_FILTER(ADM_hf_hflip)
INSTALL_VIDEO_FILTER(ADM_hf_hflip)
