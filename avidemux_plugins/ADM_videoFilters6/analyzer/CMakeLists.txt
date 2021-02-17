INCLUDE(vf_plugin)


SET(AnalyzerCommon_SRCS 
ADM_vidAnalyzer.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(AnalyzerQT_SRCS  qt4/Q_analyzer.cpp)
SET(AnalyzerQT_HEADERS  qt4/Q_analyzer.h)
SET(AnalyzerQT_UI    qt4/analyzer)
INIT_VIDEO_FILTER_QT4(ADM_vf_analyzer${QT_LIBRARY_EXTENSION} ${AnalyzerQT_SRCS} ${AnalyzerQT_HEADERS} ${AnalyzerQT_UI} ${AnalyzerCommon_SRCS} qt4/DIA_flyAnalyzer.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(AnalyzerCli_SRCS cli/DIA_analyzer.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_analyzerCli ${AnalyzerCli_SRCS} ${AnalyzerCommon_SRCS})
#
