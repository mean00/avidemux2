INCLUDE(vf_plugin)

SET(ADM_vf_aarotate_SRCS rotate.cpp)

IF(DO_COMMON)
    include(admCheckThreads)
ENDIF(DO_COMMON)

ADD_VIDEO_FILTER(ADM_vf_aarotate ${ADM_vf_aarotate_SRCS})

INIT_VIDEO_FILTER(ADM_vf_aarotate)
INSTALL_VIDEO_FILTER(ADM_vf_aarotate)

IF(DO_COMMON)
    ADM_LINK_THREAD(ADM_vf_aarotate)
ENDIF(DO_COMMON)
