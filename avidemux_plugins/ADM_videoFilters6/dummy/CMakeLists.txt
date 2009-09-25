INCLUDE(vf_plugin)


SET(ADM_vf_dummy_SRCS dummyVideoFilter.cpp)

ADD_LIBRARY(ADM_vf_dummy SHARED ${ADM_vf_dummy_SRCS})

INIT_VIDEO_FILTER(ADM_vf_dummy)
INSTALL_VIDEO_FILTER(ADM_vf_dummy)
