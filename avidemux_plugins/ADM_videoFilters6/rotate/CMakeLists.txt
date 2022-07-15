INCLUDE(vf_plugin)


SET(ADM_vf_rotate_SRCS rotate.cpp)

IF(DO_COMMON)
    include(admCheckThreads)
ENDIF(DO_COMMON)

ADD_VIDEO_FILTER(ADM_vf_rotate ${ADM_vf_rotate_SRCS})

INIT_VIDEO_FILTER(ADM_vf_rotate)
INSTALL_VIDEO_FILTER(ADM_vf_rotate)

IF(DO_COMMON)
    ADM_LINK_THREAD(ADM_vf_rotate)
ENDIF(DO_COMMON)
