INCLUDE(ve_plugin)

SET(huff_SRCS 
        huffPlugin.cpp
        ADM_huffEncoder.cpp
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(ADM_ve_huff SHARED ${huff_SRCS})

INIT_VIDEO_ENCODER(ADM_ve_huff)
INSTALL_VIDEO_ENCODER(ADM_ve_huff)
TARGET_LINK_LIBRARIES(ADM_ve_huff ADM_libavcodec6 ADM_libavutil6)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/")
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg")
