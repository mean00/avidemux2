SET(ADM_codecs_SRCS
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

ADM_ADD_SHARED_LIBRARY(ADM_coreVideoCodec6 ${ADM_codecs_SRCS})
IF(${ADM_DEBUG})
  TARGET_ADD_DEFINITIONS(ADM_coreVideoCode6 PRIVATE "-DADM_DEBUG")
ENDIF()
TARGET_LINK_LIBRARIES(ADM_coreVideoCodec6 PUBLIC
                                          ADM_core6
                                          ADM_coreImage6
                                          ADM_coreUI6
                                          ADM_coreUtils6
                                          ADM_libavcodec
                                          ADM_libavformat
                                          ADM_libavutil)

ADM_INSTALL_LIB(ADM_coreVideoCodec6)

REMOVE_DEFINITIONS(-DHAVE_CONFIG_H)
