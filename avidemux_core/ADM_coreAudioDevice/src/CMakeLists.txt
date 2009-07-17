SET(ADM_audiodevice_SRCS 
	ADM_audiodevice.cpp  
	)

ADD_LIBRARY(ADM_coreAudioDevice6 SHARED ${ADM_audiodevice_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreAudioDevice6 ADM_core6 ADM_coreUI6 ADM_coreAudio6 ADM_coreUtils6)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)
ADM_INSTALL_LIB(ADM_coreAudioDevice6)
