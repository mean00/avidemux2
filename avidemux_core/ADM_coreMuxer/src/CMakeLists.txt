
SET(ADMcoreMuxer_SRCS
ADM_dynaMuxer.cpp
ADM_muxerUtils.cpp
ADM_audioClock.cpp
ADM_coreMuxerFfmpeg.cpp
)	
#*************************************************
ADD_LIBRARY(ADM_coreMuxer6 SHARED ${ADMcoreMuxer_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_libraries/ADM_ffmpeg)
TARGET_LINK_LIBRARIES(ADM_coreMuxer6 ADM_coreImage6 ADM_coreAudio6 ADM_core6 ADM_libavcodec6 ADM_libavformat6 ADM_libavutil6)

ADM_INSTALL_LIB(ADM_coreMuxer6)
