INCLUDE(vf_plugin)

SET(ADM_vf_black_SRCS ADM_vidBlack.cpp)

ADD_VIDEO_FILTER(ADM_vf_black ${ADM_vf_black_SRCS})

INIT_VIDEO_FILTER(ADM_vf_black)
INSTALL_VIDEO_FILTER(ADM_vf_black)
