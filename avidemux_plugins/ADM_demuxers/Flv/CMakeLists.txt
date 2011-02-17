include(dm_plugin)
SET(ADM_flv_SRCS 
	ADM_flv.cpp
	ADM_flvPlugin.cpp
	ADM_flvIndex.cpp
	ADM_flvAudio.cpp)

ADD_DEMUXER(ADM_dm_flv ${ADM_flv_SRCS})

TARGET_LINK_LIBRARIES(ADM_dm_flv ADM_libavutil)

INIT_DEMUXER(ADM_dm_flv)
INSTALL_DEMUXER(ADM_dm_flv)
