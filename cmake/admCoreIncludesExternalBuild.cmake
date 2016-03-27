# eclipse
SET(CMAKE_VERBOSE_MAKEFILE ON)
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
#/
INCLUDE_DIRECTORIES(${ADM_HEADER_DIR}/avidemux_core)
#
FOREACH(inc ADM_core ADM_coreAudioFilter ADM_coreAudioParser ADM_coreAudio ADM_coreAudioDevice ADM_coreAudioEncoder/  ADM_coreDemuxer)
        INCLUDE_DIRECTORIES(${ADM_HEADER_DIR}/${inc})
ENDFOREACH()

FOREACH(inc ADM_coreImage ADM_coreMuxer ADM_coreUI ADM_coreUtils ADM_coreVideoEncoder ADM_coreVideoFilter ADM_coreVideoCodec ADM_coreImageLoader )
        INCLUDE_DIRECTORIES(${ADM_HEADER_DIR}/${inc})
ENDFOREACH()

if (UNIX)
	include_directories(${ADM_HEADER_DIR}/ADM_coreDemuxer/unix)
endif (UNIX)

