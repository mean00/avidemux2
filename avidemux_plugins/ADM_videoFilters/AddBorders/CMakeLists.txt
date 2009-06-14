INCLUDE(vf_plugin)


SET(ADM_vf_addborders_SRCS ADM_vidAddBorder.cpp)

ADD_LIBRARY(ADM_vf_addborders SHARED ${ADM_vf_addborders_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_addborders)
INSTALL_VIDEOFILTER(ADM_vf_addborders)
