INCLUDE(vf_plugin)

INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/")
SET(ADM_vf_mcdeint_SRCS ADM_vidMcDeint.cpp)

ADD_LIBRARY(ADM_vf_mcdeint SHARED ${ADM_vf_mcdeint_SRCS})
TARGET_LINK_LIBRARIES(ADM_vf_mcdeint ADM_libavcodec)

INIT_VIDEOFILTER_PLUGIN(ADM_vf_mcdeint)
INSTALL_VIDEOFILTER(ADM_vf_mcdeint)
