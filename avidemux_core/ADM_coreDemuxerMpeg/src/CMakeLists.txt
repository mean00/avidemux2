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
TARGET_LINK_LIBRARIES(ADM_coreDemuxerMpeg6 ADM_core6 ADM_coreUtils6)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include)
ADM_INSTALL_LIB(ADM_coreDemuxerMpeg6)
