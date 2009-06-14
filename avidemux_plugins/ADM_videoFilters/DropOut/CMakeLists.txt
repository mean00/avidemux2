INCLUDE(vf_plugin)


SET(ADM_vf_dropOut_SRCS ADM_vidDropOut.cpp)

ADD_LIBRARY(ADM_vf_dropOut SHARED ${ADM_vf_dropOut_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_dropOut)
INSTALL_VIDEOFILTER(ADM_vf_dropOut)
