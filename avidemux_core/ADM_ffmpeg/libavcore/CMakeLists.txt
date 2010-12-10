SET(ADM_LIB ADM_libavcore6)

SET(${ADM_LIB}_SRCS 
audioconvert.c  imgutils.c  parseutils.c  samplefmt.c  utils.c
)

ADD_LIBRARY(${ADM_LIB} SHARED ${${ADM_LIB}_SRCS})
TARGET_LINK_LIBRARIES(${ADM_LIB} ADM_core6 ADM_libavutil6)

ADM_INSTALL_LIB(${ADM_LIB} )
