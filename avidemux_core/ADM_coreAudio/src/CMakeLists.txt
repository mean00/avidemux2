
SET(ADMaudioCore_SRCS
ADM_audioUtils.cpp
ADM_audioStream.cpp
ADM_audioStreamBuffered.cpp
ADM_audioStreamMP3.cpp
ADM_audioStreamAC3.cpp
ADM_audioStreamEac3.cpp
ADM_audioStreamDCA.cpp
ADM_audioStreamPCM.cpp
ADM_audioStreamConstantChunk.cpp
ADM_audioWrite.cpp
ADM_audioWriteWav.cpp
ADM_audioIdentify.cpp
ADM_audioAccessFile.cpp
)	

add_compiler_export_flags()
ADM_ADD_SHARED_LIBRARY(ADM_coreAudio6 ${ADMaudioCore_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)
TARGET_LINK_LIBRARIES(ADM_coreAudio6 ADM_core6 ADM_coreUI6 ADM_audioParser6)

ADM_INSTALL_LIB(ADM_coreAudio6)
