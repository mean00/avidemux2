include(admAsNeeded)
include(admPluginLocation)
MACRO(INIT_MUXER _lib)
ENDMACRO()

MACRO(INSTALL_MUXER _lib)
  IF(NOT MSVC)
    SET(EXTRALIB "m")
  ENDIF()
  TARGET_LINK_LIBRARIES(${_lib} PRIVATE ADM_core6 ADM_coreUtils6 ADM_coreAudio6 ADM_coreImage6 ADM_coreUI6 ADM_core6 ADM_coreMuxer6 ${EXTRALIB} adm_pthread)
  INSTALL(TARGETS ${_lib}
                DESTINATION "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/muxers/"
                COMPONENT plugins
                )
ENDMACRO()

MACRO(ADD_MUXER name)
  ADM_ADD_SHARED_LIBRARY(${name} ${ARGN})
ENDMACRO()

