
SET(ADMcoreDemuxer_SRCS
ADM_dynaDemuxer.cpp
ADM_demuxer.cpp
)	
#*************************************************
ADD_LIBRARY(ADM_coreDemuxer SHARED ${ADMcoreDemuxer_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)
TARGET_LINK_LIBRARIES(ADM_coreDemuxer ADM_coreImage ADM_coreAudio ADM_core)

INSTALL(TARGETS ADM_coreDemuxer RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)
