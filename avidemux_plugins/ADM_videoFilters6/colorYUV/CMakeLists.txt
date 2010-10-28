INCLUDE(vf_plugin)

SET(ADM_vf_colorYuv_SRCS ADM_vidColorYuv.cpp avsColor.cpp)

ADD_VIDEO_FILTER(ADM_vf_colorYuv ${ADM_vf_colorYuv_SRCS})

INIT_VIDEO_FILTER(ADM_vf_colorYuv)
INSTALL_VIDEO_FILTER(ADM_vf_colorYuv)
