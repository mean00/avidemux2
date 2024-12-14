ADD_SUBDIRECTORY(Source)
SET(ADM_coreUtils_SRCS
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
ADM_ADD_SHARED_LIBRARY(ADM_coreUtils6 ${ADM_coreUtils_SRCS})

TARGET_INCLUDE_DIRECTORIES(ADM_coreUtils6 PUBLIC ../include)
IF(NOT MSVC)
  TARGET_COMPILE_OPTIONS(ADM_coreUtils6 PRIVATE "-std=c++11")
ENDIF()
TARGET_LINK_LIBRARIES(ADM_coreUtils6 PUBLIC ADM_json ADM_core6 ADM_coreUI6 ADM_libavutil ADM_libavcodec)


IF(MSVC)
  TARGET_COMPILE_DEFINITIONS(ADM_coreUtils6 PRIVATE "NDEBUG")
ENDIF()

ADM_INSTALL_LIB(ADM_coreUtils6)
