include(dm_plugin)
SET(ADM_ts_SRCS 
	ADM_ts.cpp
	ADM_tsPlugin.cpp
	ADM_tsIndex.cpp
	ADM_tsIndexH264.cpp
        ADM_tsIndexVC1.cpp
        ADM_tsIndexMpeg2.cpp
	ADM_tsReadIndex.cpp
	ADM_tsAudio.cpp
	ADM_tsAudioProbe.cpp
	ADM_tsAudioProbe.h
        ADM_tsComputeTimeStamp.cpp
        dmxTSPacket.cpp
        ADM_tsPatPmt.cpp
        ADM_tsBruteForce.cpp
        ADM_tsGetBits.cpp
)

ADD_DEMUXER(ADM_dm_ts ${ADM_ts_SRCS})
TARGET_LINK_LIBRARIES(ADM_dm_ts ADM_coreDemuxerMpeg6 ADM_audioParser6  ADM_libavutil ADM_libavcodec ADM_libavformat)

# For ffmpeg includes
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/") 
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_libraries") 
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_coreDemuxerMpeg/include")

INIT_DEMUXER(ADM_dm_ts)
INSTALL_DEMUXER(ADM_dm_ts)
