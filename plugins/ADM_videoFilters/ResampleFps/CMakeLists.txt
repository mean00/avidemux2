INCLUDE(vf_plugin)


SET(ADM_vf_resampleFps_SRCS ADM_vidResampleFPS.cpp)

ADD_LIBRARY(ADM_vf_resampleFps SHARED ${ADM_vf_resampleFps_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_resampleFps)
INSTALL_VIDEOFILTER(ADM_vf_resampleFps)
