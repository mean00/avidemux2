UNSET(CMAKE_CXX_VISIBILITY_PRESET)
UNSET(CMAKE_VISIBILITY_INLINES_HIDDEN)

if (USE_VDPAU)
	ADD_SUBDIRECTORY(ADM_coreVdpau)
endif (USE_VDPAU)
if (USE_XVBA)
	ADD_SUBDIRECTORY(ADM_coreXvba)
endif (USE_XVBA)
if (USE_LIBVA)
	ADD_SUBDIRECTORY(ADM_coreLibVA)
endif (USE_LIBVA)
# ADM_coreDxva2 is not built here: it lives in the GUI tree and is built by the Qt
# project (avidemux/qt4/ADM_render_qt/ADM_coreDxva2, added under IF(USE_DXVA2) by
# avidemux/qt4/ADM_render_qt/CMakeLists.txt). Nothing DXVA2-related is disabled.

SET(_CFG "${CMAKE_BINARY_DIR}/config/ADM_coreConfig.cmake")

FILE(WRITE "${_CFG}" "")

MACRO(addToConfig name)
    IF (${${name}})
        FILE(APPEND "${_CFG}" "SET(${name} 1)\n")
    ENDIF (${${name}})
ENDMACRO(addToConfig)

addToConfig("USE_VDPAU")
addToConfig("USE_XVBA")
addToConfig("USE_LIBVA")

IF(USE_LIBVA)
    addToConfig("LIBVA_HEVC_DEC")
    addToConfig("LIBVA_VP9_DEC")
    addToConfig("LIBVA_AV1_DEC")
    FILE(APPEND "${_CFG}" "SET(LIBVA_INCLUDE_DIR \"${LIBVA_INCLUDE_DIR}\")\n")
    FILE(APPEND "${_CFG}" "SET(LIBVA_CFLAGS \"${LIBVA_CFLAGS}\")\n")
    FILE(APPEND "${_CFG}" "SET(LIBVA_LIBRARY_DIR \"${LIBVA_LIBRARY_DIR}\")\n")
ENDIF(USE_LIBVA)

addToConfig("USE_DXVA2")
addToConfig("USE_NVENC")
addToConfig("USE_VIDEOTOOLBOX")
