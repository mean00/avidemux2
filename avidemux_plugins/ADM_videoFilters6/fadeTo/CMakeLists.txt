INCLUDE(vf_plugin)

SET(ADM_vf_fadeTo_SRCS ADM_vidFadeTo.cpp)

ADD_VIDEO_FILTER(ADM_vf_fadeTo ${ADM_vf_fadeTo_SRCS})

INIT_VIDEO_FILTER(ADM_vf_fadeTo)
INSTALL_VIDEO_FILTER(ADM_vf_fadeTo)
