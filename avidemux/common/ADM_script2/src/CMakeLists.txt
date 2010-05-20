SET(ADM_script_SRCS
        ADM_scriptIf.cpp
        ADM_scriptVideo.cpp
        ADM_scriptAvidemux.cpp
        ADM_scriptAvidemuxInfo.cpp
        ADM_scriptAudio.cpp
)


ADD_LIBRARY(ADM_script6 STATIC ${ADM_script_SRCS})
include_directories(../include)
include_directories(.)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_smjs)
