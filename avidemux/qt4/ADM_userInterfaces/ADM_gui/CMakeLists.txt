SET(ADM_LIB ADM_gui${QT_LIBRARY_EXTENSION})
ADD_LIBRARY(${ADM_LIB} STATIC
  Q_gui2.cpp Q_gui2GL.cpp
  Q_gui2_menu.cpp
  T_preview.cpp
  T_vumeter.cpp
  T_thumbSlider.cpp
  file_qt4.cpp gui_none.cpp ADM_mwNavSlider.cpp
  ADM_qtray.cpp
  Q_dummyWidget.cpp
  FileAction.cpp
)
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PRIVATE "${CMAKE_SOURCE_DIR}/../common/ADM_icons/")
ADM_QT_WRAP_UI(${ADM_LIB}_header gui2.ui)
ADM_QT_WRAP_CPP(${ADM_LIB}_source Q_gui2.h  ADM_mwNavSlider.h T_preview.h T_vumeter.h T_thumbSlider.h ADM_qtray.h FileAction.h)
#
# RESOURCES
#
IF(APPLE)
  ADM_QT_ADD_RESOURCES(${ADM_LIB}_resource  avidemux_osx.qrc)
ELSEIF(WIN32)
  IF(USE_CUSTOM_TIME_DISPLAY_FONT)
    ADM_QT_ADD_RESOURCES(${ADM_LIB}_resource  avidemux_win32_custom_font.qrc)
  ELSE()
    ADM_QT_ADD_RESOURCES(${ADM_LIB}_resource  avidemux_win32.qrc)
  ENDIF()
ELSE()
  ADM_QT_ADD_RESOURCES(${ADM_LIB}_resource  avidemux.qrc)
ENDIF()
#
IF(USE_CUSTOM_TIME_DISPLAY_FONT)
  TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PRIVATE "USE_CUSTOM_TIME_DISPLAY_FONT")
ENDIF()

IF(USE_OPENGL)
  TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_openGL${QT_LIBRARY_EXTENSION}6)
ENDIF()

IF(WIN32)
  TARGET_SOURCES(${ADM_LIB} PRIVATE ADM_systemTrayProgress_win32.cpp)
ELSE()
  TARGET_SOURCES(${ADM_LIB} PRIVATE ADM_systemTrayProgress_default.cpp)
ENDIF()

TARGET_SOURCES(${ADM_LIB} PRIVATE ${${ADM_LIB}_header}
  ${${ADM_LIB}_source}  ${${ADM_LIB}_resource})

TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../ADM_filters
  ${CMAKE_CURRENT_SOURCE_DIR}/../../qlementine/lib/include
)

TARGET_LINK_LIBRARIES(${ADM_LIB} PRIVATE ADM_editor6_interface adm_pthread ADM_qtx_${QT_EXTENSION})
TARGET_LINK_LIBRARIES(${ADM_LIB} PRIVATE  ADM_commonUI6_interface)

