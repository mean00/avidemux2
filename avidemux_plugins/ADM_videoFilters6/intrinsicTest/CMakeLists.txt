INCLUDE(vf_plugin)


SET(ADM_vf_intrinsicTest_SRCS intrinsicTestVideoFilter.cpp)

ADD_VIDEO_FILTER(ADM_vf_intrinsicTest ${ADM_vf_intrinsicTest_SRCS})

INIT_VIDEO_FILTER(ADM_vf_intrinsicTest)
INSTALL_VIDEO_FILTER(ADM_vf_intrinsicTest)
