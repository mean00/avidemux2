INCLUDE(vf_plugin)


SET(QuadTransCommon_SRCS 
ADM_vidQuadTrans.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(QuadTransQT_SRCS  qt4/Q_quadTrans.cpp)
SET(QuadTransQT_HEADERS  qt4/Q_quadTrans.h)
SET(QuadTransQT_UI    qt4/quadTrans)
INIT_VIDEO_FILTER_QT4(ADM_vf_quadTrans${QT_LIBRARY_EXTENSION} ${QuadTransQT_SRCS} ${QuadTransQT_HEADERS} ${QuadTransQT_UI} ${QuadTransCommon_SRCS} qt4/DIA_flyQuadTrans.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(QuadTransCli_SRCS cli/DIA_quadTrans.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_quadTransCli ${QuadTransCli_SRCS} ${QuadTransCommon_SRCS})
#
