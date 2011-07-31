INCLUDE(vf_plugin)


SET(ADM_vf_removePlane_SRCS removePlane.cpp)

ADD_VIDEO_FILTER(ADM_vf_removePlane ${ADM_vf_removePlane_SRCS})

INIT_VIDEO_FILTER(ADM_vf_removePlane)
INSTALL_VIDEO_FILTER(ADM_vf_removePlane)
