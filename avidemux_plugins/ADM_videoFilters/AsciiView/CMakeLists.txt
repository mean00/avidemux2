###############################################
INCLUDE(vf_plugin)

SET(ADM_vf_ascii_SRCS ADM_vidAscii.cpp)

ADD_VIDEO_FILTER(ADM_vf_ascii ${ADM_vf_ascii_SRCS})
INIT_VIDEO_FILTER(ADM_vf_ascii)
INSTALL_VIDEO_FILTER(ADM_vf_ascii)
