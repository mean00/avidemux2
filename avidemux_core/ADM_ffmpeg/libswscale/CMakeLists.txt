SET(ADM_LIB ADM_libswscale6)
SET(${ADM_LIB}_SRCS 
        swscale.c yuv2rgb.c rgb2rgb.c ADM_mp.cpp swscale_avoption.c
)

IF(ADM_CPU_X86)
SET(${ADM_LIB}_SRCS ${${ADM_LIB}_SRCS} x86/yuv2rgb_mmx.c)
ENDIF(ADM_CPU_X86)

IF (ADM_CPU_ALTIVEC)
	SET(${ADM_LIB}_SRCS ${${ADM_LIB}_SRCS} yuv2rgb_altivec.c)
ENDIF (ADM_CPU_ALTIVEC)

ADD_LIBRARY(${ADM_LIB} SHARED ${${ADM_LIB}_SRCS})

IF (APPLE AND ADM_DEBUG)
	ADD_TARGET_CFLAGS(${ADM_LIB} -O2)
ENDIF (APPLE AND ADM_DEBUG)

TARGET_LINK_LIBRARIES(${ADM_LIB} ADM_core6 ADM_libavutil6)

ADM_INSTALL_LIB(${ADM_LIB} )
