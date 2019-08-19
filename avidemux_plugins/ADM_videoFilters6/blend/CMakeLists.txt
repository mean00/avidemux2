INCLUDE(vf_plugin)

SET(ADM_vf_blend_SRCS ADM_vidBlendFrames.cpp)

ADD_VIDEO_FILTER(ADM_vf_blend ${ADM_vf_blend_SRCS})

INIT_VIDEO_FILTER(ADM_vf_blend)
INSTALL_VIDEO_FILTER(ADM_vf_blend)
