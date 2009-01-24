INCLUDE(vf_plugin)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg/ADM_libpostproc")

SET(ADM_vf_forcedPP_SRCS ADM_vidForcedPP.cpp)

ADD_LIBRARY(ADM_vf_forcedPP SHARED ${ADM_vf_forcedPP_SRCS})
TARGET_LINK_LIBRARIES(ADM_vf_forcedPP ADM_libpostproc)

INIT_VIDEOFILTER_PLUGIN(ADM_vf_forcedPP)
INSTALL_VIDEOFILTER(ADM_vf_forcedPP)
