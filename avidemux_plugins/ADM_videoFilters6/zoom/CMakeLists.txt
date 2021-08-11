# Zoom filter
INCLUDE(vf_plugin)

SET(ZoomCommon_SRCS ADM_vidZoom.cpp  )

# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
IF(DO_QT4)
  IF(ADM_QT_VERSION EQUAL 4)
    SET(QT_TAG qt4)
  ELSE(ADM_QT_VERSION EQUAL 4)
      SET(QT_TAG qt5)
  ENDIF(ADM_QT_VERSION EQUAL 4)
  SET(ADM_vf_zoomQT4_SRCS    ${QT_TAG}/Q_zoom.cpp)
  SET(ADM_vf_zoomQT4_Headers ${QT_TAG}/Q_zoom.h)
  SET(ADM_vf_zoomQT4_UI      ${QT_TAG}/zoom)
  INIT_VIDEO_FILTER_QT4(ADM_vf_zoom${QT_LIBRARY_EXTENSION} ${ADM_vf_zoomQT4_SRCS} ${ADM_vf_zoomQT4_Headers} ${ADM_vf_zoomQT4_UI} ${ZoomCommon_SRCS} 
        ${QT_TAG}/DIA_flyZoom.cpp)
ENDIF(DO_QT4)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ZoomCli_SRCS cli/DIA_zoom.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_ZoomCli ${ZoomCli_SRCS} ${ZoomCommon_SRCS})
#

