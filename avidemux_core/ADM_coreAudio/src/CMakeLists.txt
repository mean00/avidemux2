
SET(ADM_LIB ADM_coreAudio6)
ADM_ADD_SHARED_LIBRARY(${ADM_LIB})
TARGET_SOURCES( ${ADM_LIB} PRIVATE
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
  ADM_audioWriteAAC.cpp
  ADM_audioIdentify.cpp
  ADM_audioAccessFile.cpp
  ADM_audioAccessFileAACADTS.cpp
  ADM_audioClock.cpp
  ADM_audioXiphUtils.cpp
)

ADD_LIBRARY(${ADM_LIB}_interface INTERFACE)
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB}_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_core6 ADM_coreUI6 ADM_audioParser6)
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

ADM_INSTALL_LIB(${ADM_LIB})
