#
# Low level demuxers for mpeg TS/PTS/ES/MS
#
#

SET(ADMcoreDemuxerMpeg_SRCS
ADM_indexFile.cpp
dmx_io.cpp
dmxPacket.cpp
dmxPSPacket.cpp
)
#*************************************************
ADM_ADD_SHARED_LIBRARY(ADM_coreDemuxerMpeg6 ${ADMcoreDemuxerMpeg_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreDemuxerMpeg6 PUBLIC ADM_core6 ADM_coreUtils6 ADM_coreDemuxer6)
TARGET_INCLUDE_DIRECTORIES(ADM_coreDemuxerMpeg6 PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/../include)
ADM_INSTALL_LIB(ADM_coreDemuxerMpeg6)
