
SET(ADMaudioCore_SRCS
ADM_audioUtils.cpp
ADM_audioStream.cpp
ADM_audioStreamBuffered.cpp
ADM_audioStreamMP3.cpp
ADM_audioStreamAC3.cpp
)	
#*************************************************
#*************************************************
ADD_LIBRARY(ADM_coreAudio SHARED ${ADMaudioCore_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)
TARGET_LINK_LIBRARIES(ADM_coreAudio ADM_core ADM_coreUI ADM_audioParser)

ADM_INSTALL_LIB(ADM_coreAudio)
