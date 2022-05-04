INCLUDE(vf_plugin)

SET(ADM_vf_decimateFrame_SRCS ADM_vidDecimateFrame.cpp)

ADD_VIDEO_FILTER(ADM_vf_decimateFrame ${ADM_vf_decimateFrame_SRCS})

INIT_VIDEO_FILTER(ADM_vf_decimateFrame)
INSTALL_VIDEO_FILTER(ADM_vf_decimateFrame)
