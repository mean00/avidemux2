SET(ADM_LIB ADM_gui2Gtk)

SET(${ADM_LIB}_SRCS
	GUI_bindings.cpp  GUI_gtkRender.cpp  GUI_keymap.cpp  GUI_glade.cpp)

IF (APPLE)
	SET(${ADM_LIB}_SRCS ${${ADM_LIB}_SRCS} GUI_gtkRenderHelper.m)
ENDIF (APPLE)

ADD_LIBRARY(${ADM_LIB} STATIC  ${${ADM_LIB}_SRCS})
ADD_DEFINITIONS("-DTARGET_DIR='\"${CMAKE_INSTALL_PREFIX}/lib/ADM_glade/\"' -DSOURCE_DIR='\"${CMAKE_CURRENT_SOURCE_DIR}\"'")
