INCLUDE(vf_plugin)


SET(ADM_vf_blackenBorders_SRCS ADM_vidBlackenBorders.cpp)

ADD_LIBRARY(ADM_vf_blackenBorders SHARED ${ADM_vf_blackenBorders_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_blackenBorders)
INSTALL_VIDEOFILTER(ADM_vf_blackenBorders)
