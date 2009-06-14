INCLUDE(vf_plugin)


SET(ADM_vf_telecide_SRCS ADM_vidDecTelecide.cpp  videoFunctions.cpp)

ADD_LIBRARY(ADM_vf_telecide SHARED ${ADM_vf_telecide_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_telecide)
INSTALL_VIDEOFILTER(ADM_vf_telecide)
