SET(ADM_LIB ADM_filters${QT_LIBRARY_EXTENSION})

ADM_QT_WRAP_UI(${ADM_LIB}_header  mainfilter.ui  seekablePreview.ui quickfilter.ui)
ADM_QT_WRAP_CPP(${ADM_LIB}_source  Q_mainfilter.h  Q_seekablePreview.h)
ADM_QT_ADD_RESOURCES(${ADM_LIB}_resource  filter.qrc)

SET(${ADM_LIB}_SRCS
	filter_none.cpp  
        Q_mainfilter.cpp 
#        Q_partialize.cpp 
        Q_seekablePreview.cpp 
        ${${ADM_LIB}_header}  ${${ADM_LIB}_source}  ${${ADM_LIB}_resource})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
