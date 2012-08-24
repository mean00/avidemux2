
SET(ADMcoreMuxer_SRCS
ADM_dynaMuxer.cpp
ADM_muxerUtils.cpp
ADM_audioClock.cpp
ADM_coreMuxerFfmpeg.cpp
)	

add_compiler_export_flags()
ADM_ADD_SHARED_LIBRARY(ADM_coreMuxer6 ${ADMcoreMuxer_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)

TARGET_LINK_LIBRARIES(ADM_coreMuxer6 ADM_coreImage6 ADM_coreAudio6 ADM_core6 ADM_libavcodec ADM_libavformat ADM_libavutil ADM_coreUtils6)

if (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)
	add_dependencies(ADM_coreMuxer6 libavcodec libavformat libavutil)
endif (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)

ADM_INSTALL_LIB(ADM_coreMuxer6)
