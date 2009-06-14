SET(ADM_LIB ADM_dialogGuiNone)

SET(${ADM_LIB}_SRCS 
	file_none.cpp  gui_none.cpp  preview_none.cpp)

ADD_ADM_LIB_CLI_TARGET(${ADM_LIB} ${${ADM_LIB}_SRCS})