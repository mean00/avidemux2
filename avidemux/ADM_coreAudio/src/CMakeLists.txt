
SET(ADMaudioCore_SRCS
ADM_audioUtils.cpp
audioencoder.cpp
ADM_audioFilter.cpp
ADM_audioResample.cpp
ADM_audioStream.cpp
ADM_audioStreamBuffered.cpp
ADM_audioStreamMP3.cpp
ADM_audioStreamAC3.cpp
ADM_libsamplerate/samplerate.c
ADM_libsamplerate/src_linear.c
ADM_libsamplerate/src_sinc.c
ADM_libsamplerate/src_zoh.c
)	
#*************************************************
ADD_DEFINITIONS( "-DADM_LEGACY_PROGGY")
ADD_DEFINITIONS( "-DHAVE_LRINT -DHAVE_LRINTF ")
ADD_DEFINITIONS( "-DCPU_CLIPS_POSITIVE=0 -DCPU_CLIPS_NEGATIVE=0")
ADD_DEFINITIONS( "-I${CMAKE_CURRENT_SOURCE_DIR} ")
ADD_DEFINITIONS( "-DPACKAGE=`"SRC`"")
ADD_DEFINITIONS( "-DVERSION=`"0.1.4`"")
#*************************************************
ADD_LIBRARY(ADM_coreAudio SHARED ${ADMaudioCore_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)
TARGET_LINK_LIBRARIES(ADM_coreAudio ADM_core ADM_coreUI)

INSTALL(TARGETS ADM_coreAudio RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)
