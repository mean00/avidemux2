SET(ADM_LIB ADM_filtersGtk)

SET(${ADM_LIB}_SRCS
	gui_filtermanager.cpp  gui_filtermanager_dialog.cpp)

ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
FILE(GLOB ADM_filterIcons ${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common/ADM_icons/videoFilter/*.png)
INSTALL(FILES videoFilter.gtkBuilder ${ADM_filterIcons} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ADM_glade/videoFilter")
