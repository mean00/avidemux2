SET(ADM_scriptDF_SRCS
        ADM_JSDFMenu.cpp
        ADM_JSDFToggle.cpp
        ADM_JSDFInteger.cpp
        ADM_JSDialogFactory.cpp
        ADM_JSDialogFactory_if.cpp 
        
)

ADD_DEFINITIONS("-DJS_THREADSAFE -DXP_UNIX")
ADD_LIBRARY(ADM_scriptDF STATIC ${ADM_scriptDF_SRCS})
include_directories(../include)
include_directories(.)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_smjs)
