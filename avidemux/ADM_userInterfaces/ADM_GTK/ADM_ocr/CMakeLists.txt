SET(ADM_LIB ADM_ocrGtk)

SET(${ADM_LIB}_SRCS
	adm_ocr.cpp
	adm_editglyph.cpp)

ADD_ADM_LIB_GTK_TARGET(${ADM_LIB} ${${ADM_LIB}_SRCS})
ADD_CFLAGS_GTK_TARGET(${ADM_LIB} ${GTK_CFLAGS})