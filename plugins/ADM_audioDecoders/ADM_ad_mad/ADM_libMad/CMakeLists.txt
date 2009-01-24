SET(ADM_LIB ADM_libMad)

SET(${ADM_LIB}_SRCS 
	bit.c  decoder.c  fixed.c  frame.c  huffman.c  layer12.c  layer3.c  
	stream.c  synth.c  timer.c  version.c)

ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})

IF (UNIX)
	ADD_TARGET_CFLAGS(${ADM_LIB} -fPIC)
ENDIF (UNIX)

IF (ADM_BIG_ENDIAN)
	ADD_TARGET_CFLAGS(${ADM_LIB} "-DWORDS_BIGENDIAN")
ENDIF (ADM_BIG_ENDIAN)

INIT_AUDIO_PLUGIN(${ADM_LIB})
TARGET_LINK_LIBRARIES(${ADM_LIB} ADM_core)