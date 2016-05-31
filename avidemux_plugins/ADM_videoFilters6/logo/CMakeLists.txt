INCLUDE(vf_plugin)
SET(logoCommon_SRCS  ADM_vidLogo.cpp  )
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(logoQT_SRCS         qt4/Q_logo.cpp )
SET(logoQT_HEADERS      qt4/Q_logo.h)
SET(logoQT_UI           qt4/logo)
IF(DO_QT4)
  INIT_VIDEO_FILTER_QT4(ADM_vf_logo${QT_LIBRARY_EXTENSION} ${logoQT_SRCS} ${logoQT_HEADERS} ${logoQT_UI} ${logoCommon_SRCS} qt4/DIA_flyLogo.cpp)
  TARGET_LINK_LIBRARIES(ADM_vf_logo${QT_LIBRARY_EXTENSION} ADM_coreImageLoader6)
ENDIF(DO_QT4)

#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(logoCli_SRCS cli/logo_cli.cpp)
IF(DO_CLI)
  INIT_VIDEO_FILTER_CLI(  ADM_vf_logoCli ${logoCli_SRCS} ${logoCommon_SRCS})
  TARGET_LINK_LIBRARIES(ADM_vf_logoCli ADM_coreImageLoader6)
ENDIF(DO_CLI)

