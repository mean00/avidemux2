SET(ADM_audiodevice_SRCS
  ADM_audiodevice.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreAudioDevice6 ${ADM_audiodevice_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreAudioDevice6 PRIVATE ADM_core6 ADM_coreUI6 ADM_coreAudio6 ADM_coreUtils6)
TARGET_INCLUDE_DIRECTORIES(ADM_coreAudioDevice6 PRIVATE ${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include
                    "${PTHREAD_INCLUDE_DIR}")
ADM_INSTALL_LIB(ADM_coreAudioDevice6)
