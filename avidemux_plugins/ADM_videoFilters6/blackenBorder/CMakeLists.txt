INCLUDE(vf_plugin)


SET(ADM_vf_blackenBorders_SRCS ADM_vidBlackBorder.cpp )

ADD_VIDEO_FILTER(ADM_vf_blackenBorders ${ADM_vf_blackenBorders_SRCS})

INIT_VIDEO_FILTER(ADM_vf_blackenBorders)
INSTALL_VIDEO_FILTER(ADM_vf_blackenBorders)
