INCLUDE(vf_plugin)


SET(ADM_vf_logo_SRCS ADM_vidLogo.cpp)

ADD_LIBRARY(ADM_vf_logo SHARED ${ADM_vf_logo_SRCS})

INIT_VIDEO_FILTER(ADM_vf_logo)
INSTALL_VIDEO_FILTER(ADM_vf_logo)
