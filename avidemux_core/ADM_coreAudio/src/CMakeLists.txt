
SET(ADMaudioCore_SRCS
ADM_audioUtils.cpp
ADM_audioStream.cpp
ADM_audioStreamBuffered.cpp
ADM_audioStreamMP3.cpp
ADM_audioStreamAC3.cpp
ADM_audioStreamEac3.cpp
ADM_audioStreamConstantChunk.cpp
)	
#*************************************************
#*************************************************
ADD_LIBRARY(ADM_coreAudio6 SHARED ${ADMaudioCore_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)
TARGET_LINK_LIBRARIES(ADM_coreAudio6 ADM_core6 ADM_coreUI6 ADM_audioParser6)

ADM_INSTALL_LIB(ADM_coreAudio6)
