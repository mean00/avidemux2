SET(ADM_LIB ADM_gui2Gtk)

SET(${ADM_LIB}_SRCS
	GUI_bindings.cpp  GUI_gtkRender.cpp  GUI_keymap.cpp  GUI_main2.cpp)

IF (APPLE)
	SET(${ADM_LIB}_SRCS ${${ADM_LIB}_SRCS} GUI_gtkRenderHelper.m)
ENDIF (APPLE)

ADD_ADM_LIB_GTK_TARGET(${ADM_LIB} ${${ADM_LIB}_SRCS})
ADD_CFLAGS_GTK_TARGET(${ADM_LIB} ${GTK_CFLAGS})
