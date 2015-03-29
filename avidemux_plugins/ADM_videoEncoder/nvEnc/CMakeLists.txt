INCLUDE(ve_plugin)

SET(nvenc_SRCS 
        nvEncPlugin.cpp
        nvEnc.cpp
        nvEnc_utils.cpp
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_VIDEO_ENCODER(ADM_ve_nvenc  ${nvenc_SRCS})

INIT_VIDEO_ENCODER(ADM_ve_nvenc)
INSTALL_VIDEO_ENCODER(ADM_ve_nvenc)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/")
