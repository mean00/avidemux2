
SET(ADM_LIB ADM_coreVideoCodec6)
ADM_ADD_SHARED_LIBRARY(${ADM_LIB})
TARGET_SOURCES(${ADM_LIB} PRIVATE
  ADM_codecs.cpp
  ADM_ffmp43.cpp
  ADM_codecFFsimple.cpp
  ADM_rgb16.cpp
  ADM_uyvy.cpp
  ADM_codecSearch.cpp
  DIA_lavDecoder.cpp
  ADM_ffHwAccel.cpp
  ADM_ffVp9.cpp
)

TARGET_LINK_LIBRARIES(ADM_coreVideoCodec6 PUBLIC
                                          ADM_core6
                                          ADM_coreImage6
                                          ADM_coreUI6
                                          ADM_coreUtils6
                                          ADM_coreImage6
                                          ADM_libavcodec
                                          ADM_libavformat
                                          ADM_libavutil
)

TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

ADM_INSTALL_LIB(${ADM_LIB})

IF(${ADM_DEBUG})
  TARGET_COMPILE_DEFINITIONS(ADM_coreVideoCodec6 PRIVATE "ADM_DEBUG")
ENDIF()

REMOVE_DEFINITIONS(-DHAVE_CONFIG_H)
