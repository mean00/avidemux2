INCLUDE(ve_plugin)

SET(utvideo_SRCS
        utvideoEncPlugin.cpp
        ADM_utvideoEncoder.cpp
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_VIDEO_ENCODER(ADM_ve_utvideo ${utvideo_SRCS})

INIT_VIDEO_ENCODER(ADM_ve_utvideo)
INSTALL_VIDEO_ENCODER(ADM_ve_utvideo)
TARGET_LINK_LIBRARIES(ADM_ve_utvideo ADM_libavcodec ADM_libavutil)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/")
