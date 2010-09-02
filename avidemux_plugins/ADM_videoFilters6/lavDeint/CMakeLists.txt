INCLUDE(vf_plugin)


SET(ADM_vf_lavDeint_SRCS lavDeint.cpp)

ADD_VIDEO_FILTER(ADM_vf_lavDeint ${ADM_vf_lavDeint_SRCS})

INIT_VIDEO_FILTER(ADM_vf_lavDeint)
INSTALL_VIDEO_FILTER(ADM_vf_lavDeint)
