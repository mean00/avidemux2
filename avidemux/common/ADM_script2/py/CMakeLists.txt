SET(ADM_script_SRCS
        ADM_pyAdm.cpp
)

ADD_LIBRARY(ADM_pyScript6 STATIC ${ADM_script_SRCS})
ADD_DEFINITIONS(-DCPYTHON_MOD)
include_directories(../include)
include_directories(.)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_coreTinyPy/include)
