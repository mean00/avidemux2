SET(ADM_LIB ADM_commonUI6_${QT_EXTENSION})
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
ADD_LIBRARY(ADM_commonUI6_interface INTERFACE)
TARGET_INCLUDE_DIRECTORIES(ADM_commonUI6_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(${ADM_LIB}  PRIVATE ADM_editor6_interface ADM_core6 adm_gettext ADM_qtx_${QT_EXTENSION})
TARGET_LINK_LIBRARIES(${ADM_LIB}  PRIVATE ADM_ui_interface ADM_coreAudioDevice6 ADM_render6)


