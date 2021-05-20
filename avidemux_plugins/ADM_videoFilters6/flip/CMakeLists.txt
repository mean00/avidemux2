INCLUDE(vf_plugin)


SET(FlipCommon_SRCS 
ADM_vidFlip.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(FlipQT_SRCS  qt4/Q_flip.cpp)
SET(FlipQT_HEADERS  qt4/Q_flip.h)
SET(FlipQT_UI    qt4/flip)
INIT_VIDEO_FILTER_QT4(ADM_vf_flip${QT_LIBRARY_EXTENSION} ${FlipQT_SRCS} ${FlipQT_HEADERS} ${FlipQT_UI} ${FlipCommon_SRCS} qt4/DIA_flyFlip.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(FlipCli_SRCS cli/DIA_flip.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_flipCli ${FlipCli_SRCS} ${FlipCommon_SRCS})
#
