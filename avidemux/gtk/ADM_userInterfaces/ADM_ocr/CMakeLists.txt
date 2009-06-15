SET(ADM_LIB ADM_ocrGtk)

SET(${ADM_LIB}_SRCS
	adm_ocr.cpp
	adm_editglyph.cpp)

ADD_LIBRARY(${ADM_LIB} STATIC  ${${ADM_LIB}_SRCS})
