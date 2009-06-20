SET(ADM_LIB ADM_UIGtk)

SET(${ADM_LIB}_SRCS
	DIA_color.cpp
	DIA_filesel.cpp
	DIA_dialogFactory.cpp
	FAC_bitrate.cpp
	FAC_float.cpp
	FAC_hex.cpp
	FAC_menu.cpp
	FAC_readOnlyText.cpp
	FAC_bar.cpp
	FAC_button.cpp
	FAC_frame.cpp
	FAC_integer.cpp
	FAC_notch.cpp
	FAC_toggle.cpp
	FAC_threadCount.cpp
	FAC_slider.cpp
	FAC_matrix.cpp
        ADM_gladeSupport.cpp  
        choice.cpp       
        toolkit_dialog.cpp
        toolkit.cpp
        DIA_flyDialogGtk.cpp
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include)
ADD_LIBRARY(${ADM_LIB} SHARED ${${ADM_LIB}_SRCS})
TARGET_LINK_LIBRARIES(${ADM_LIB} ${GTK_LDFLAGS} ADM_core ADM_coreUI ADM_render_gtk)
ADD_TARGET_CFLAGS(${ADM_LIB} ${GTK_CFLAGS})
ADD_SOURCE_CFLAGS(DIA_flyDialogGtk.cpp " -DADM_UI_TYPE_BUILD=2")
ADM_INSTALL_LIB( ${ADM_LIB})
