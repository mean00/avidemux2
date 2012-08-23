SET(ADM_audiodevice_SRCS 
	ADM_audiodevice.cpp  
	)

add_compiler_export_flags()
ADM_ADD_SHARED_LIBRARY(ADM_coreAudioDevice6 ${ADM_audiodevice_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreAudioDevice6 ADM_core6 ADM_coreUI6 ADM_coreAudio6 ADM_coreUtils6)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include "${PTHREAD_INCLUDE_DIR}")
ADM_INSTALL_LIB(ADM_coreAudioDevice6)
