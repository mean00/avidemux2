SET(ADM_LIB ADM_commonUI6)
ADD_LIBRARY(${ADM_LIB} STATIC)
TARGET_SOURCES(${ADM_LIB} PRIVATE
  DIA_audioFilter.cpp
  DIA_gototime.cpp
  DIA_postproc.cpp
  #DIA_bitrateHisto.cpp
  DIA_prefs.cpp
  DIA_builtin.cpp
  DIA_jobs_save.cpp
  DIA_plugins.cpp
  DIA_HDRConfig.cpp
)

TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_core6 ADM_coreUtils6 ADM_editor6_interface ADM_core6)
#
ADD_LIBRARY(${ADM_LIB}_interface INTERFACE)
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB}_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
