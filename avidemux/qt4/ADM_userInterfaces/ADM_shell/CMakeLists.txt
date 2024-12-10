SET(ADM_LIB ADM_shell${QT_LIBRARY_EXTENSION})

SET(
    uiFiles
  shell.ui
)
SET(headers
  Q_shell.h
)

ADM_QT_WRAP_UI(${ADM_LIB}_headers ${uiFiles})
ADM_QT_WRAP_CPP(${ADM_LIB}_source ${headers})

SET(${ADM_LIB}_SRCS
  ${${ADM_LIB}_headers}
  ${${ADM_LIB}_source}
  Q_shell.cpp
  qt4Shell.cpp
)

ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PRIVATE "${CMAKE_SOURCE_DIR}/avidemux/ADM_UIs/ADM_QT4/include")
TARGET_LINK_LIBRARIES(${ADM_LIB} PRIVATE ADM_editor6_interface)
