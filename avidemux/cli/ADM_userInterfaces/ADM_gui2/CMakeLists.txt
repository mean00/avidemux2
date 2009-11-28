SET(ADM_LIB ADM_gui2Cli6)

SET(${ADM_LIB}_SRCS 
	file_none.cpp  gui_none.cpp  preview_none.cpp)

ADD_LIBRARY(${ADM_LIB} SHARED ${${ADM_LIB}_SRCS})
