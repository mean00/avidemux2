INCLUDE(vf_plugin)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)

SET(ADM_vf_resampleFps_SRCS ADM_vidResampleFPS.cpp motin.cpp)


#ADD_VIDEO_FILTER(ADM_vf_resampleFps ${ADM_vf_resampleFps_SRCS})

ADD_LIBRARY(ADM_vf_resampleFps SHARED ${ADM_vf_resampleFps_SRCS})
INCLUDE(vf_plugin)

INIT_VIDEO_FILTER(ADM_vf_resampleFps)
INSTALL_VIDEO_FILTER(ADM_vf_resampleFps)
ADD_TARGET_CFLAGS(ADM_vf_resampleFps "-I${AVIDEMUX_SOURCE_DIR}/avidemux_plugins")
