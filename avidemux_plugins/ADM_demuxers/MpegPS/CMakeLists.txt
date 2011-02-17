include(dm_plugin)
SET(ADM_ps_SRCS 
	ADM_ps.cpp
	ADM_psPlugin.cpp
	ADM_psIndex.cpp
	ADM_psReadIndex.cpp
	ADM_psAudio.cpp
	ADM_psAudioProbe.cpp
	ADM_psAudioProbe.h
        ADM_psComputeTimeStamp.cpp
)

ADD_DEMUXER(ADM_dm_ps ${ADM_ps_SRCS})
TARGET_LINK_LIBRARIES(ADM_dm_ps ADM_coreDemuxerMpeg6 ADM_audioParser6)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_coreDemuxerMpeg/include")



INIT_DEMUXER(ADM_dm_ps)
INSTALL_DEMUXER(ADM_dm_ps)
