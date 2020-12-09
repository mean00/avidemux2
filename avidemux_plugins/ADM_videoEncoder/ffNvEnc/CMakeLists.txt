INCLUDE(ve_plugin)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

MACRO(DEFINE_VID_ENCODER codec)
    SET(family ffNvenc)
    SET(outName ADM_ve_${family}${codec})
    SET(${family}${codec}_SRCS
        ffNvEncPlugin.cpp
        ADM_ffNvEnc.cpp)
    ADD_VIDEO_ENCODER(${outName} ${${family}${codec}_SRCS})
    IF("${codec}" STREQUAL "HEVC")
        TARGET_COMPILE_DEFINITIONS(${outName} PUBLIC H265_ENCODER)
    ENDIF("${codec}" STREQUAL "HEVC")
    INSTALL_VIDEO_ENCODER(${outName})
    TARGET_LINK_LIBRARIES(${outName} ADM_libavcodec ADM_libavutil)
ENDMACRO(DEFINE_VID_ENCODER codec)

DEFINE_VID_ENCODER(H264)
DEFINE_VID_ENCODER(HEVC)
