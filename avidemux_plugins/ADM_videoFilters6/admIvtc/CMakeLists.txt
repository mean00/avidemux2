INCLUDE(vf_plugin)

SET(ADM_vf_admIvtc_SRCS ADM_admIvtc.cpp ADM_admIvtc_main.cpp ADM_admIvtc_util.cpp)

ADD_VIDEO_FILTER(ADM_vf_admIvtc ${ADM_vf_admIvtc_SRCS})

INIT_VIDEO_FILTER(ADM_vf_admIvtc)
INSTALL_VIDEO_FILTER(ADM_vf_admIvtc)
