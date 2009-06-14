SET(ADM_ocr_SRCS 
	adm_glyph.cpp  adm_leftturn.cpp  ADM_vob2vobsub.cpp
	ADM_ocr.cpp
	ADM_bitmapSource.cpp
	ADM_ocrEngine.cpp)

ADD_ADM_LIB_ALL_TARGETS(ADM_ocr ${ADM_ocr_SRCS})