INCLUDE(vf_plugin)

SET(ADM_vf_resampleFps_SRCS ADM_vidResampleFPS.cpp motin.cpp)

IF(DO_COMMON)
    include(admCheckThreads)
ENDIF(DO_COMMON)

ADD_VIDEO_FILTER(ADM_vf_resampleFps ${ADM_vf_resampleFps_SRCS})
INIT_VIDEO_FILTER(ADM_vf_resampleFps)
INSTALL_VIDEO_FILTER(ADM_vf_resampleFps)

IF(DO_COMMON)
    ADM_LINK_THREAD(ADM_vf_resampleFps)
ENDIF(DO_COMMON)

