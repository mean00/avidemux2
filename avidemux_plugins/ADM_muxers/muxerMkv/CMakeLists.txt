include(mx_plugin)
SET(ADM_Mkv_SRCS 
	muxerMkv.cpp
	muxerMkvPlugin.cpp
        muxerMkvConfig.cpp
)

ADD_LIBRARY(ADM_mx_Mkv SHARED ${ADM_Mkv_SRCS})

INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg")
TARGET_LINK_LIBRARIES(ADM_mx_Mkv  ADM_libavutil6 ADM_libavcodec6 ADM_libavformat6)
INIT_MUXER(ADM_mx_Mkv)
INSTALL_MUXER(ADM_mx_Mkv)

