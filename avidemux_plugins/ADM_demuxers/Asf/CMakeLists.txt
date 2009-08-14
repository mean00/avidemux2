include(dm_plugin)
SET(ADM_asf_SRCS 
	ADM_asfAudio.cpp
        ADM_asfChunk.cpp  
        ADM_asf.cpp  
        ADM_asfPacket.cpp
        ADM_asfPlugin.cpp
)

ADD_LIBRARY(ADM_dm_asf SHARED ${ADM_asf_SRCS})

#TARGET_LINK_LIBRARIES(ADM_dm_asf ADM_libavutil6)

INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg")

INIT_DEMUXER(ADM_dm_asf)
INSTALL_DEMUXER(ADM_dm_asf)
