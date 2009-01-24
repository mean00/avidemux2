INCLUDE(vf_plugin)


SET(ADM_vf_unblend_SRCS ADM_vidUnblend.cpp)

ADD_LIBRARY(ADM_vf_unblend SHARED ${ADM_vf_unblend_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_unblend)
INSTALL_VIDEOFILTER(ADM_vf_unblend)
