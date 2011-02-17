INCLUDE(ve_plugin)

SET(huff_SRCS 
        huffPlugin.cpp
        ADM_huffEncoder.cpp
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_VIDEO_ENCODER(ADM_ve_huff  ${huff_SRCS})

INIT_VIDEO_ENCODER(ADM_ve_huff)
INSTALL_VIDEO_ENCODER(ADM_ve_huff)
TARGET_LINK_LIBRARIES(ADM_ve_huff ADM_libavcodec ADM_libavutil)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/")
