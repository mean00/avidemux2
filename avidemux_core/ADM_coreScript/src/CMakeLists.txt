SET(admScript_SRCS
  ADM_scriptDFInteger.cpp
  ADM_scriptDFFloat.cpp
  ADM_scriptDFMenu.cpp
  ADM_scriptDFToggle.cpp
  ADM_scriptDialogFactory.cpp
  ADM_ScriptEngineLoader.cpp
  ADM_scriptDFTimeStamp.cpp
  ADM_scriptDFReadOnlyText.cpp
  ADM_scriptDFText.cpp
)

ADM_ADD_SHARED_LIBRARY(ADM_coreScript ${admScript_SRCS})
ADM_INSTALL_LIB(ADM_coreScript)

TARGET_INCLUDE_DIRECTORIES(ADM_coreScript PUBLIC ../include "${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common")

TARGET_LINK_LIBRARIES(ADM_coreScript PUBLIC ADM_coreUtils6)
