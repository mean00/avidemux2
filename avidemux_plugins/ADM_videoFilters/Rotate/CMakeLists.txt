INCLUDE(vf_plugin)


SET(ADM_vf_rotate_SRCS ADM_vidRotate.cpp)

ADD_LIBRARY(ADM_vf_rotate SHARED ${ADM_vf_rotate_SRCS})

INIT_VIDEOFILTER_PLUGIN(ADM_vf_rotate)
INSTALL_VIDEOFILTER(ADM_vf_rotate)
