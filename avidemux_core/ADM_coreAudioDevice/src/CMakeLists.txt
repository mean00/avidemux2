SET(ADM_audiodevice_SRCS 
	ADM_audiodevice.cpp  
	)

ADD_LIBRARY(ADM_coreAudioDevice SHARED ${ADM_audiodevice_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreAudioDevice ADM_core ADM_coreUI ADM_coreAudio ADM_coreUtils)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)
INSTALL(TARGETS ADM_coreAudioDevice RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)
