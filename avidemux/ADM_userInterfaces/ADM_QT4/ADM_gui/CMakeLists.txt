SET(ADM_LIB ADM_guiQt4)

QT4_WRAP_UI(${ADM_LIB}_header  gui2.ui )
QT4_WRAP_CPP(${ADM_LIB}_source  Q_gui2.h  T_preview.h)
QT4_ADD_RESOURCES(${ADM_LIB}_resource  avidemux.qrc)

SET(${ADM_LIB}_SRCS
	Q_gui2.cpp  
        T_preview.cpp  
        #stubs.cpp
        file_qt4.cpp  gui_none.cpp  ADM_qslider.cpp
	${${ADM_LIB}_header}  ${${ADM_LIB}_source}  ${${ADM_LIB}_resource})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../ADM_filters  )
ADD_ADM_LIB_QT4_TARGET(${ADM_LIB} ${${ADM_LIB}_SRCS})
