include(mx_plugin)

MACRO(DEFINE_MUXER ext)
    SET(ADM_${ext}_SRCS
        muxerMkv.cpp
        muxerMkvPlugin.cpp
        muxerMkvConfig.cpp)

    ADD_MUXER(ADM_mx_${ext} ${ADM_${ext}_SRCS})

    IF("${ext}" STREQUAL "webm")
        TARGET_COMPILE_DEFINITIONS(ADM_mx_${ext} PUBLIC MUXER_IS_WEBM)
    ENDIF("${ext}" STREQUAL "webm")

    TARGET_LINK_LIBRARIES(ADM_mx_${ext}
        ADM_libavutil
        ADM_libavcodec
        ADM_libavformat)

    INIT_MUXER(ADM_mx_${ext})
    INSTALL_MUXER(ADM_mx_${ext})
ENDMACRO(DEFINE_MUXER ext)

DEFINE_MUXER(mkv)
DEFINE_MUXER(webm)
