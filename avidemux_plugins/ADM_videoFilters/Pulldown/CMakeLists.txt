INCLUDE(vf_plugin)


SET(ADM_vf_Pulldown_SRCS ADM_vidPulldown.cpp)

ADD_LIBRARY(ADM_vf_Pulldown SHARED ${ADM_vf_Pulldown_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_Pulldown)
INSTALL_VIDEOFILTER(ADM_vf_Pulldown)
