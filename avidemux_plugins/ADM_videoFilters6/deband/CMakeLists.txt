INCLUDE(vf_plugin)


SET(DebandCommon_SRCS 
ADM_vidDeband.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(DebandQT_SRCS  qt4/Q_deband.cpp)
SET(DebandQT_HEADERS  qt4/Q_deband.h)
SET(DebandQT_UI    qt4/deband)
INIT_VIDEO_FILTER_QT4(ADM_vf_deband${QT_LIBRARY_EXTENSION} ${DebandQT_SRCS} ${DebandQT_HEADERS} ${DebandQT_UI} ${DebandCommon_SRCS} qt4/DIA_flyDeband.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(DebandCli_SRCS cli/DIA_deband.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_debandCli ${DebandCli_SRCS} ${DebandCommon_SRCS})
#
