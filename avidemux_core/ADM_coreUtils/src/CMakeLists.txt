ADD_SUBDIRECTORY(Source)
SET(ADM_coreUtils_SRCS 
ADM_confCouple.cpp  
ADM_bitstream.cpp 
avidemutils.cpp  
ADM_quota.cpp
fourcc.cpp
ADM_infoExtractor.cpp
ADM_infoExtractorH264.cpp
ADM_infoExtractorH265.cpp
ADM_frameType.cpp
ADM_codecType.cpp
ADM_file.cpp
ADM_paramList.cpp
ADM_coreCodecMapping.cpp
ADM_threadQueue.cpp
ADM_string.cpp
ADM_getbits.cpp
ADM_writeRiff.cpp
prefs.cpp
prefs2_json.cpp
ADM_memio.cpp
ADM_json.cpp
ADM_iso639.cpp
)

include_directories(../include Source "${PTHREAD_INCLUDE_DIR}")

add_compiler_export_flags()
ADM_ADD_SHARED_LIBRARY(ADM_coreUtils6 ${ADM_coreUtils_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreUtils6 ADM_json ADM_core6 ADM_coreUI6 ADM_libavutil ADM_libavcodec )

if (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)
	add_dependencies(ADM_coreUtils6 libavcodec libavutil)
endif (NOT ${CMAKE_VERSION} VERSION_GREATER 2.8.3)

if (MSVC)
	add_definitions(-DNDEBUG)
endif (MSVC)

ADM_INSTALL_LIB(ADM_coreUtils6)
