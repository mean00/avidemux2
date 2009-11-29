INCLUDE(vf_plugin)


SET(ADM_vf_dummy_SRCS dummyVideoFilter.cpp)

ADD_VIDEO_FILTER(ADM_vf_dummy ${ADM_vf_dummy_SRCS})

INIT_VIDEO_FILTER(ADM_vf_dummy)
INSTALL_VIDEO_FILTER(ADM_vf_dummy)
