INCLUDE(vf_plugin)


SET(ColorTempCommon_SRCS 
ADM_vidColorTemp.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ColorTempQT_SRCS  qt4/Q_colorTemp.cpp)
SET(ColorTempQT_HEADERS  qt4/Q_colorTemp.h)
SET(ColorTempQT_UI    qt4/colorTemp)
INIT_VIDEO_FILTER_QT4(ADM_vf_colorTemp${QT_LIBRARY_EXTENSION} ${ColorTempQT_SRCS} ${ColorTempQT_HEADERS} ${ColorTempQT_UI} ${ColorTempCommon_SRCS} qt4/DIA_flyColorTemp.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(ColorTempCli_SRCS cli/DIA_colorTemp.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_colorTempCli ${ColorTempCli_SRCS} ${ColorTempCommon_SRCS})
#
