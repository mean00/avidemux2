include(mx_plugin)

MACRO(DEFINE_MUXER ext)
    SET(ADM_${ext}_SRCS
        muxerMP4.cpp
        muxerMP4Plugin.cpp
        muxerMP4Config.cpp)

    ADD_MUXER(ADM_mx_${ext} ${ADM_${ext}_SRCS})

    IF("${ext}" STREQUAL "mov")
        TARGET_COMPILE_DEFINITIONS(ADM_mx_${ext} PUBLIC MUXER_IS_MOV)
    ENDIF("${ext}" STREQUAL "mov")

    TARGET_LINK_LIBRARIES(ADM_mx_${ext}
        ADM_libavutil
        ADM_libavcodec
        ADM_libavformat)

    INIT_MUXER(ADM_mx_${ext})
    INSTALL_MUXER(ADM_mx_${ext})
ENDMACRO(DEFINE_MUXER ext)

DEFINE_MUXER(mp4)
DEFINE_MUXER(mov)
