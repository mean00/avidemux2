INCLUDE(vf_plugin)


SET(ADM_vf_lumaOnly_SRCS lumaOnly.cpp)

ADD_VIDEO_FILTER(ADM_vf_lumaOnly ${ADM_vf_lumaOnly_SRCS})

INIT_VIDEO_FILTER(ADM_vf_lumaOnly)
INSTALL_VIDEO_FILTER(ADM_vf_lumaOnly)
