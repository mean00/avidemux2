SET(ADM_script_SRCS
        ADM_scriptIf.cpp
        ADM_scriptVideo.cpp
        ADM_scriptAvidemux.cpp
        ADM_scriptAvidemuxInfo.cpp
        ADM_scriptAudio.cpp
        ADM_scriptUtils.cpp
        scriptDialogFactory/ADM_scriptDFInteger.cpp
        scriptDialogFactory/ADM_scriptDFMenu.cpp
        scriptDialogFactory/ADM_scriptDFToggle.cpp
        scriptDialogFactory/ADM_scriptDialogFactory.cpp
)


ADD_LIBRARY(ADM_script6 STATIC ${ADM_script_SRCS})
include_directories(../include)
include_directories(.)
include_directories(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_smjs)
