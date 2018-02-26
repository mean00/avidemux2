INCLUDE(vf_plugin)

SET(ADM_vf_stillimage_SRCS stillimage.cpp)

ADD_VIDEO_FILTER(ADM_vf_stillimage ${ADM_vf_stillimage_SRCS})

INIT_VIDEO_FILTER(ADM_vf_stillimage)
INSTALL_VIDEO_FILTER(ADM_vf_stillimage)
