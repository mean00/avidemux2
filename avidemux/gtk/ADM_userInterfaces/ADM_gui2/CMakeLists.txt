SET(ADM_LIB ADM_gui2Gtk)

SET(${ADM_LIB}_SRCS
	GUI_bindings.cpp  GUI_gtkRender.cpp  GUI_keymap.cpp  )

IF (APPLE)
	SET(${ADM_LIB}_SRCS ${${ADM_LIB}_SRCS} GUI_gtkRenderHelper.m)
ENDIF (APPLE)

ADD_LIBRARY(${ADM_LIB} STATIC  ${${ADM_LIB}_SRCS})
