INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
SET(ADM_LIB ADM_libavutil6)

SET(${ADM_LIB}_SRCS 
opt.c
cpu.c
eval.c
adler32.c
aes.c
base64.c
crc.c
des.c
fifo.c
integer.c
intfloat_readwrite.c
lls.c
log.c
lzo.c
mathematics.c
md5.c
#random.c
random_seed.c
rational.c
rc4.c
#string.c
avstring.c
tree.c
lfg.c
mem.c
pixdesc.c
sha.c
)
IF(ADM_CPU_X86)
	SET(${ADM_LIB}_SRCS  ${${ADM_LIB}_SRCS}
			x86/cpu.c
				)
ENDIF(ADM_CPU_X86)

ADD_LIBRARY(${ADM_LIB} SHARED ${${ADM_LIB}_SRCS})
TARGET_LINK_LIBRARIES(${ADM_LIB} ADM_core6)

ADM_INSTALL_LIB(${ADM_LIB} )
