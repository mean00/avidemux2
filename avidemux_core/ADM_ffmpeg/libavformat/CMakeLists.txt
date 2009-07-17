SET(ADM_LIB ADM_libavformat6)

SET(${ADM_LIB}_SRCS 
	mpeg.c  utils.c  avio.c  aviobuf.c  cutils.c  img2.c  file.c  mpegtsenc.c
	movenc.c  avienc.c  wav.c  mov.c  matroska.c  isom.c  isom.h  
	flvenc.c  matroskaenc.c metadata.c metadata_compat.c options.c
        flacenc.c
	avc.c raw.c  riff.c)

ADD_LIBRARY(${ADM_LIB} SHARED ${${ADM_LIB}_SRCS})
TARGET_LINK_LIBRARIES(${ADM_LIB} ADM_core6 ADM_libavutil6 ADM_libavcodec6)

ADM_INSTALL_LIB(${ADM_LIB} )
