
SET(ADMcoreMuxer_SRCS
ADM_dynaMuxer.cpp
ADM_videoCopy.cpp
ADM_muxerUtils.cpp
ADM_audioClock.cpp
)	
#*************************************************
ADD_LIBRARY(ADM_coreMuxer SHARED ${ADMcoreMuxer_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg)
TARGET_LINK_LIBRARIES(ADM_coreMuxer ADM_coreImage ADM_coreAudio ADM_core)

INSTALL(TARGETS ADM_coreMuxer RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)
