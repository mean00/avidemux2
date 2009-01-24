SET(ADM_nuv_SRCS 
	ADM_nuvAudio.cpp  ADM_nuv.cpp  minilzo.c  RTjpeg.c  RTjpegN.cpp  RToldformat.cpp)

ADD_ADM_LIB_ALL_TARGETS(ADM_nuv ${ADM_nuv_SRCS})

IF (ADM_BIG_ENDIAN)
	ADD_CFLAGS_ALL_TARGETS(ADM_nuv "-DWORDS_BIGENDIAN")
ENDIF (ADM_BIG_ENDIAN)