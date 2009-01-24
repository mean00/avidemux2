INCLUDE(vf_plugin)


SET(ADM_vf_Stabilize_SRCS ADM_vidStabilize.cpp)

ADD_LIBRARY(ADM_vf_Stabilize SHARED ${ADM_vf_Stabilize_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_Stabilize)
INSTALL_VIDEOFILTER(ADM_vf_Stabilize)
