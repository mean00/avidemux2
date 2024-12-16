
SET(ADM_LIB ADM_coreUtils6)
ADM_ADD_SHARED_LIBRARY(${ADM_LIB})
TARGET_SOURCES(${ADM_LIB} PRIVATE
  ADM_confCouple.cpp
  ADM_bitstream.cpp
  avidemutils.cpp
  ADM_quota.cpp
  ADM_memFile.cpp
  fourcc.cpp
  ADM_extraDataExtractor.cpp
  ADM_infoExtractor.cpp
  ADM_infoExtractorH264.cpp
  ADM_infoExtractorH265.cpp
  ADM_infoExtractorVC1.cpp
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
  ADM_last.cpp
)

ADD_SUBDIRECTORY(Source)

TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_json ADM_core6 ADM_coreUI6 ADM_libavutil ADM_libavcodec)
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

ADM_INSTALL_LIB(${ADM_LIB})

IF(MSVC)
  TARGET_COMPILE_DEFINITIONS(ADM_coreUtils6 PRIVATE "NDEBUG")
ELSE()
  TARGET_COMPILE_OPTIONS(ADM_coreUtils6 PRIVATE "-std=c++11")
ENDIF()


