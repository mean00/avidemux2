
SET(ADMaudioCoreEncoder_SRCS
audioencoder.cpp
)	
#*************************************************
#*************************************************
ADM_ADD_SHARED_LIBRARY(ADM_coreAudioEncoder6 ${ADMaudioCoreEncoder_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreAudioEncoder6 ADM_core6 ADM_coreAudio6 ADM_coreAudioFilterAPI6 ADM_coreUI6)

ADM_INSTALL_LIB(ADM_coreAudioEncoder6)
