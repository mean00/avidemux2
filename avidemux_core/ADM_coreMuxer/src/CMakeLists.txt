
SET(ADMcoreMuxer_SRCS
ADM_dynaMuxer.cpp
ADM_muxerUtils.cpp
ADM_coreMuxerFfmpeg.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreMuxer6 ${ADMcoreMuxer_SRCS})
TARGET_INCLUDE_DIRECTORIES(ADM_coreMuxer6 PUBLIC ${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)

TARGET_LINK_LIBRARIES(ADM_coreMuxer6 PUBLIC ADM_coreImage6 ADM_coreAudio6 ADM_core6 ADM_libavcodec ADM_libavformat ADM_libavutil ADM_coreUtils6)

ADM_INSTALL_LIB(ADM_coreMuxer6)
