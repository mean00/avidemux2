INCLUDE(vf_plugin)


SET(ADM_vf_negative_SRCS ADM_negative.cpp)

ADD_VIDEO_FILTER(ADM_vf_negative ${ADM_vf_negative_SRCS})

INIT_VIDEO_FILTER(ADM_vf_negative)
INSTALL_VIDEO_FILTER(ADM_vf_negative)
