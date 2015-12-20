SET(ADM_LIB ADM_gui${QT_LIBRARY_EXTENSION})
INCLUDE_DIRECTORIES("${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common/ADM_icons/")
ADM_QT_WRAP_UI(${ADM_LIB}_header  gui2.ui )
ADM_QT_WRAP_CPP(${ADM_LIB}_source  Q_gui2.h  T_preview.h T_vumeter.h T_thumbSlider.h ADM_qtray.h FileAction.h)
ADM_QT_ADD_RESOURCES(${ADM_LIB}_resource  avidemux.qrc)

IF(USE_OPENGL)
        INCLUDE_DIRECTORIES(../../ADM_openGL/include/)
ENDIF(USE_OPENGL)

IF(WIN32)
    SET(TRAYPROGRESS    ADM_systemTrayProgress_win32.cpp)
ELSE(WIN32)
    SET(TRAYPROGRESS    ADM_systemTrayProgress_default.cpp)
ENDIF(WIN32)

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
        FileAction.cpp
        ${TRAYPROGRESS}
	${${ADM_LIB}_header}  ${${ADM_LIB}_source}  ${${ADM_LIB}_resource})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../ADM_filters  )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../ADM_filters  )
ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
