include(mx_plugin)
SET(ADM_Mkv_SRCS 
	muxerMkv.cpp
	muxerMkvPlugin.cpp
        muxerMkvConfig.cpp
)

ADD_LIBRARY(ADM_mx_Mkv SHARED ${ADM_Mkv_SRCS})

TARGET_LINK_LIBRARIES(ADM_mx_Mkv  ADM_libavutil ADM_libavcodec ADM_libavformat)
INIT_MUXER(ADM_mx_Mkv)
INSTALL_MUXER(ADM_mx_Mkv)

