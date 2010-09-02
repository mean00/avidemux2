INCLUDE(vf_plugin)


SET(ADM_vf_rotate_SRCS rotate.cpp)

ADD_VIDEO_FILTER(ADM_vf_rotate ${ADM_vf_rotate_SRCS})

INIT_VIDEO_FILTER(ADM_vf_rotate)
INSTALL_VIDEO_FILTER(ADM_vf_rotate)
