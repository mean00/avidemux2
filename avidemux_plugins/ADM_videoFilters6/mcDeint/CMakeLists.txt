INCLUDE(vf_plugin)

SET(ADM_vf_mcdeint_SRCS ADM_vidMcDeint.cpp)

ADD_VIDEO_FILTER(ADM_vf_mcdeint ${ADM_vf_mcdeint_SRCS})

INIT_VIDEO_FILTER(ADM_vf_mcdeint)
INSTALL_VIDEO_FILTER(ADM_vf_mcdeint)
TARGET_LINK_LIBRARIES( ADM_vf_mcdeint ADM_libavcodec ADM_libavutil)
