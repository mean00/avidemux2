SET(ADM_LIB ADM_UIQT4)

QT4_WRAP_CPP(${ADM_LIB}_source
	T_bitrate.h  T_button.h  T_dialogFactory.h  T_filesel.h  T_menu.h  T_slider.h
	T_threadCount.h  T_toggle.h  T_notch.h)

SET(${ADM_LIB}_T
	T_bitrate.cpp  T_button.cpp  T_dialogFactory.cpp  T_filesel.cpp  T_menu.cpp
	T_slider.cpp  T_threadCount.cpp  T_toggle.cpp  T_notch.cpp
    T_flyDialogQt4.cpp T_QCanvas.cpp
)

SET(${ADM_LIB}_SRCS
	FAC_bar.cpp    FAC_frame.cpp  FAC_integer.cpp  FAC_aspectRatio.cpp
	FAC_float.cpp  FAC_hex.cpp    FAC_readOnlyText.cpp  toolkit.cpp
	FAC_matrix.cpp  DIA_none.cpp  ${${ADM_LIB}_source}  ${${ADM_LIB}_T})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include)
ADD_LIBRARY(${ADM_LIB} SHARED ${${ADM_LIB}_SRCS})
TARGET_LINK_LIBRARIES(${ADM_LIB} ADM_core6 ADM_coreUI6 ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ADM_render6_qt4)
#INSTALL(TARGETS ${ADM_LIB} RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)
ADM_INSTALL_LIB( ${ADM_LIB} )
