SET(ADM_script_SRCS
        ADM_jsIf.cpp
        ADM_jsDebug.cpp
        ADM_jsUtils.cpp
# Wrapper code
        ADM_jsIf_js.c
# Factory test
        ADM_jsTestFactory.cpp  
        ADM_jsTestFactory_js.c
# Load segment
        ADM_jsAvidemux.cpp
        ADM_jsAvidemux_js.c
# Debug / editor class
        ADM_jsEditor.cpp
        ADM_jsEditor_js.c

# 
        ADM_jsVideo.cpp
#
        ADM_jsAudio.cpp
)

ADD_LIBRARY(ADM_jsScript6 STATIC ${ADM_script_SRCS})
ADD_DEFINITIONS("-DJS_THREADSAFE -DXP_UNIX")
include_directories(../include)
include_directories(.)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_smjs)
SUBDIRS(src_dialogFactory)
