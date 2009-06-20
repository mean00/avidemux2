
SET(ADMcoreDemuxer_SRCS
ADM_dynaDemuxer.cpp
ADM_demuxer.cpp
)	
#*************************************************
ADD_LIBRARY(ADM_coreDemuxer SHARED ${ADMcoreDemuxer_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)
TARGET_LINK_LIBRARIES(ADM_coreDemuxer ADM_coreImage ADM_coreAudio ADM_core)

ADM_INSTALL_LIB(ADM_coreDemuxer)
