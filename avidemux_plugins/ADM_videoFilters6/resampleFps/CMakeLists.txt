INCLUDE(vf_plugin)

SET(ADM_vf_resampleFps_SRCS ADM_vidResampleFPS.cpp)

ADD_VIDEO_FILTER(ADM_vf_resampleFps ${ADM_vf_resampleFps_SRCS})

INIT_VIDEO_FILTER(ADM_vf_resampleFps)
INSTALL_VIDEO_FILTER(ADM_vf_resampleFps)
