INCLUDE(vf_plugin)


SET(ADM_vf_swscaleResize_SRCS swScaleResize.cpp)

ADD_LIBRARY(ADM_vf_swscaleResize SHARED ${ADM_vf_swscaleResize_SRCS})

INIT_VIDEO_FILTER(ADM_vf_swscaleResize)
INSTALL_VIDEO_FILTER(ADM_vf_swscaleResize)
