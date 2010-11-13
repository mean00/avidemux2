include(dm_plugin)
SET(ADM_ts_SRCS 
	ADM_ts.cpp
	ADM_tsPlugin.cpp
	ADM_tsIndex.cpp
	ADM_tsIndexH264.cpp
	ADM_tsReadIndex.cpp
	ADM_tsAudio.cpp
	ADM_tsAudioProbe.cpp
	ADM_tsAudioProbe.h
        ADM_tsComputeTimeStamp.cpp
        dmxTSPacket.cpp
        ADM_tsPatPmt.cpp
        ADM_tsGetBits.cpp
)

ADD_LIBRARY(ADM_dm_ts SHARED ${ADM_ts_SRCS})
TARGET_LINK_LIBRARIES(ADM_dm_ts ADM_coreDemuxerMpeg6 ADM_audioParser6  ADM_libavutil6 ADM_libavcodec6 ADM_libavformat6)
# For ffmpeg includes
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/") 
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries") 
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg") 
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_coreDemuxerMpeg/include")

INIT_DEMUXER(ADM_dm_ts)
INSTALL_DEMUXER(ADM_dm_ts)
