SET(ADM_LIB ADM_filtersGtk)

SET(${ADM_LIB}_SRCS
	gui_filtermanager.cpp  gui_filtermanager_dialog.cpp)

ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
