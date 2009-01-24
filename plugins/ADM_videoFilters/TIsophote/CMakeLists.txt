INCLUDE(vf_plugin)


SET(ADM_vf_Tisophote_SRCS ADM_vidTisophote.cpp)

ADD_LIBRARY(ADM_vf_Tisophote SHARED ${ADM_vf_Tisophote_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_Tisophote)
INSTALL_VIDEOFILTER(ADM_vf_Tisophote)
