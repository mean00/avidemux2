INCLUDE(vf_plugin)


SET(ADM_vf_printInfo_SRCS printInfo.cpp)

ADD_VIDEO_FILTER(ADM_vf_printInfo ${ADM_vf_printInfo_SRCS})

INIT_VIDEO_FILTER(ADM_vf_printInfo)
INSTALL_VIDEO_FILTER(ADM_vf_printInfo)
