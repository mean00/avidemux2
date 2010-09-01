INCLUDE(vf_plugin)


SET(ADM_vf_logo_SRCS ADM_vidLogo.cpp)

ADD_VIDEO_FILTER(ADM_vf_logo ${ADM_vf_logo_SRCS})
TARGET_LINK_LIBRARIES(ADM_vf_logo ADM_coreImageLoader6 )
INIT_VIDEO_FILTER(ADM_vf_logo)
INSTALL_VIDEO_FILTER(ADM_vf_logo)
