INCLUDE(vf_plugin)
SET(logoCommon_SRCS  ADM_vidLogo.cpp  )
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(logoQT_SRCS         qt4/Q_logo.cpp )
SET(logoQT_HEADERS      qt4/Q_logo.h)
SET(logoQT_UI           qt4/logo)
IF(DO_QT4)
  INIT_VIDEO_FILTER_QT4(ADM_vf_logo${QT_LIBRARY_EXTENSION} ${logoQT_SRCS} ${logoQT_HEADERS} ${logoQT_UI} ${logoCommon_SRCS} )
  TARGET_LINK_LIBRARIES(ADM_vf_logo${QT_LIBRARY_EXTENSION} PRIVATE ADM_coreImageLoader6)
ENDIF()

#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(logoCli_SRCS cli/logo_cli.cpp)
IF(DO_CLI)
  INIT_VIDEO_FILTER_CLI(  ADM_vf_logoCli ${logoCli_SRCS} ${logoCommon_SRCS})
  TARGET_LINK_LIBRARIES(ADM_vf_logoCli PRIVATE ADM_coreImageLoader6)
ENDIF()

