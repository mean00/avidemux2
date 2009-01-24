SET(ADM_LIB ADM_dialogNone)

SET(${ADM_LIB}_SRCS 
	alert_none.cpp  DIA_busy.cpp  
#DIA_encoding.cpp   i
        DIA_indexing.cpp  DIA_none.cpp  DIA_working.cpp  OCR_none.cpp)

ADD_ADM_LIB_CLI_TARGET(${ADM_LIB} ${${ADM_LIB}_SRCS})
