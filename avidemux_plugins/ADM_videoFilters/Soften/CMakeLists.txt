INCLUDE(vf_plugin)


SET(ADM_vf_soften_SRCS ADM_vidSoften.cpp)

ADD_LIBRARY(ADM_vf_soften SHARED ${ADM_vf_soften_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_soften)
INSTALL_VIDEOFILTER(ADM_vf_soften)
