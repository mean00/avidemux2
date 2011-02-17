include(mx_plugin)
SET(ADM_ffPS_SRCS 
	muxerffPS.cpp
	muxerffPSPlugin.cpp
        muxerffPSConfig.cpp
)

ADD_MUXER(ADM_mx_ffPS ${ADM_ffPS_SRCS})

TARGET_LINK_LIBRARIES(ADM_mx_ffPS  ADM_libavutil ADM_libavcodec ADM_libavformat)
INIT_MUXER(ADM_mx_ffPS)
INSTALL_MUXER(ADM_mx_ffPS)

