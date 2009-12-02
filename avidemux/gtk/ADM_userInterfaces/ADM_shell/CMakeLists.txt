SET(ADM_LIB ADM_shellGtk)

SET(${ADM_LIB}_SRCS 
        gtkShell.cpp
)

INCLUDE_DIRECTORIES("${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common/ADM_script/")
ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
