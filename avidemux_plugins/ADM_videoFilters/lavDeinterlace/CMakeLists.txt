INCLUDE(vf_plugin)


SET(ADM_vf_lavDeinterlace_SRCS ADM_lavpp_deint.cpp)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/")
ADD_LIBRARY(ADM_vf_lavDeinterlace SHARED ${ADM_vf_lavDeinterlace_SRCS})
TARGET_LINK_LIBRARIES(ADM_vf_lavDeinterlace ADM_libswscale ADM_libpostproc)

INIT_VIDEOFILTER_PLUGIN(ADM_vf_lavDeinterlace)
INSTALL_VIDEOFILTER(ADM_vf_lavDeinterlace)
