
SET(ADM_LIB ADM_coreMuxer6)
ADM_ADD_SHARED_LIBRARY(${ADM_LIB})
TARGET_SOURCES(${ADM_LIB} PRIVATE
  ADM_dynaMuxer.cpp
  ADM_muxerUtils.cpp
  ADM_coreMuxerFfmpeg.cpp)

TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_coreImage6 ADM_coreAudio6 ADM_core6 ADM_libavcodec ADM_libavformat ADM_libavutil ADM_coreUtils6)
ADM_INSTALL_LIB(${ADM_LIB}) # API
#
