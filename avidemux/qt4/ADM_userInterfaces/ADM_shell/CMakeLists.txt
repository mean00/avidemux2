SET(ADM_LIB ADM_shellQt4)

SET(    uiFiles
        shell.ui  
        )
SET(headers
	Q_shell.h
)

QT4_WRAP_UI(${ADM_LIB}_headers ${uiFiles})
QT4_WRAP_CPP(${ADM_LIB}_source ${headers})

SET(${ADM_LIB}_SRCS 
	${${ADM_LIB}_headers}  ${${ADM_LIB}_source} 
        Q_shell.cpp  
        qt4Shell.cpp
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/avidemux/ADM_UIs/ADM_QT4/include")
INCLUDE_DIRECTORIES("${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common/ADM_script2/include/")
ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
