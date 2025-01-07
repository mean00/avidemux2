SET(ADM_LIB ADM_filters${QT_LIBRARY_EXTENSION})

ADM_QT_WRAP_UI(${ADM_LIB}_header  mainfilter.ui  seekablePreview.ui quickfilter.ui)
ADM_QT_WRAP_CPP(${ADM_LIB}_source  Q_mainfilter.h  Q_seekablePreview.h)
ADM_QT_ADD_RESOURCES(${ADM_LIB}_resource  filter.qrc)

ADD_LIBRARY(${ADM_LIB} STATIC)
TARGET_SOURCES(${ADM_LIB} PRIVATE
  filter_none.cpp
  Q_mainfilter.cpp
  Q_seekablePreview.cpp
  ${${ADM_LIB}_header} ${${ADM_LIB}_source} ${${ADM_LIB}_resource}
)

TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
TARGET_LINK_LIBRARIES(${ADM_LIB} PRIVATE ADM_videoFilter6_interface ADM_coreVideoFilter6 ADM_editor6_interface ADM_render6_interface ADM_commonUI6_interface  adm_pthread)
