INCLUDE(vf_plugin)

SET(FitToSizeCommon_SRCS ADM_vidFitToSize.cpp)


# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(FitToSizeQT4_SRCS    qt4/Q_fitToSize.cpp)
SET(FitToSizeQT4_Headers qt4/Q_fitToSize.h)
SET(FitToSizeQT4_UI      qt4/fitToSize)
INIT_VIDEO_FILTER_QT4(ADM_vf_fitToSize${QT_LIBRARY_EXTENSION} ${FitToSizeQT4_SRCS} ${FitToSizeQT4_Headers} ${FitToSizeQT4_UI} ${FitToSizeCommon_SRCS})
IF(DO_QT4)
  TARGET_LINK_LIBRARIES(ADM_vf_fitToSize${QT_LIBRARY_EXTENSION} PRIVATE ADM_libswscale)
ENDIF()
# /QT4

# ----------------- CLI Version -----------------------
INCLUDE(vf_plugin_cli)
SET(FitToSizeCli_SRCS    cli/DIA_fitToSize.cpp)
INIT_VIDEO_FILTER_CLI(ADM_vf_fitToSizeCli ${FitToSizeCli_SRCS} ${FitToSizeCommon_SRCS})
IF(DO_CLI)
  TARGET_LINK_LIBRARIES(ADM_vf_fitToSizeCli PRIVATE ADM_libswscale)
ENDIF()

# /QT4
