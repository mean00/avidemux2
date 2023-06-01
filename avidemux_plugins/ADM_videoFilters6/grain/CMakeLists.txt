INCLUDE(vf_plugin)


SET(GrainCommon_SRCS 
ADM_vidGrain.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(GrainQT_SRCS  qt4/Q_grain.cpp)
SET(GrainQT_HEADERS  qt4/Q_grain.h)
SET(GrainQT_UI    qt4/grain)
INIT_VIDEO_FILTER_QT4(ADM_vf_grain${QT_LIBRARY_EXTENSION} ${GrainQT_SRCS} ${GrainQT_HEADERS} ${GrainQT_UI} ${GrainCommon_SRCS} qt4/DIA_flyGrain.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(GrainCli_SRCS cli/DIA_grain.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_grainCli ${GrainCli_SRCS} ${GrainCommon_SRCS})
#
