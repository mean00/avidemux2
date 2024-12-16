SET(ADM_LIB ADM_coreScript)
ADM_ADD_SHARED_LIBRARY(${ADM_LIB})
TARGET_SOURCES(${ADM_LIB} PRIVATE
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

TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_core6 ADM_coreUtils6)
ADM_INSTALL_LIB(${ADM_LIB}) # API
#
