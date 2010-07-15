INCLUDE(vf_plugin)


SET(ADM_vf_addBorders_SRCS ADM_vidAddBorder.cpp)

ADD_VIDEO_FILTER(ADM_vf_addBorders ${ADM_vf_addBorders_SRCS})

INIT_VIDEO_FILTER(ADM_vf_addBorders)
INSTALL_VIDEO_FILTER(ADM_vf_addBorders)
