INCLUDE(vf_plugin)

SET(ADM_vf_fadeToBlack_SRCS ADM_vidFade.cpp)

ADD_VIDEO_FILTER(ADM_vf_fadeToBlack ${ADM_vf_fadeToBlack_SRCS})

INIT_VIDEO_FILTER(ADM_vf_fadeToBlack)
INSTALL_VIDEO_FILTER(ADM_vf_fadeToBlack)
