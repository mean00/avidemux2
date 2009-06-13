
SET(ADMaudioCoreEncoder_SRCS
audioencoder.cpp
)	
#*************************************************
#*************************************************
ADD_LIBRARY(ADM_coreAudioEncoder SHARED ${ADMaudioCoreEncoder_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreAudioEncoder ADM_core ADM_coreAudio ADM_coreAudioFilterAPI ADM_coreUI)

INSTALL(TARGETS ADM_coreAudioEncoder RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)
