SET(ADM_LIB ADM_guiQt4)
INCLUDE_DIRECTORIES("${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common/ADM_icons/")
QT4_WRAP_UI(${ADM_LIB}_header  gui2.ui )
QT4_WRAP_CPP(${ADM_LIB}_source  Q_gui2.h  T_preview.h T_vumeter.h T_thumbSlider.h ADM_qtray.h)
QT4_ADD_RESOURCES(${ADM_LIB}_resource  avidemux.qrc)

SET(${ADM_LIB}_SRCS
	Q_gui2.cpp   Q_gui2GL.cpp
	Q_gui2_menu.cpp  
        T_preview.cpp  
        T_vumeter.cpp  
        #stubs.cpp
        T_thumbSlider.cpp
        file_qt4.cpp  gui_none.cpp  ADM_qslider.cpp
	ADM_qtray.cpp
        Q_dummyWidget.cpp
	${${ADM_LIB}_header}  ${${ADM_LIB}_source}  ${${ADM_LIB}_resource})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../ADM_filters  )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../ADM_filters  )
ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
