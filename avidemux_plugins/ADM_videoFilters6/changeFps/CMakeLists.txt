INCLUDE(vf_plugin)

SET(ADM_vf_changeFps_SRCS changeFps.cpp)

ADD_VIDEO_FILTER(ADM_vf_changeFps ${ADM_vf_changeFps_SRCS})

INIT_VIDEO_FILTER(ADM_vf_changeFps)
INSTALL_VIDEO_FILTER(ADM_vf_changeFps)
