
SET(ADM_LIB ADM_coreVideoEncoder6 )
ADM_ADD_SHARED_LIBRARY(${ADM_LIB})
TARGET_SOURCES(${ADM_LIB} PRIVATE
  ADM_coreVideoEncoder.cpp
  ADM_coreVideoEncoderFFmpeg.cpp
)

TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC
  ADM_core6 ADM_coreUtils6 ADM_coreImage6 ADM_libavcodec ADM_libavutil)
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

ADM_INSTALL_LIB(${ADM_LIB})



