INCLUDE(vf_plugin)

SET(AiEnhanceCommon_SRCS ADM_vidAiEnhance.cpp NeuronSW.cpp FSRCNN.cpp)
IF(DO_QT4)
    include(admCheckThreads)
ENDIF(DO_QT4)
IF(DO_CLI)
    include(admCheckThreads)
ENDIF(DO_CLI)

# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(AiEnhanceQT_SRCS  qt4/Q_aiEnhance.cpp)
SET(AiEnhanceQT_HEADERS  qt4/Q_aiEnhance.h)
SET(AiEnhanceQT_UI    qt4/aiEnhance)
INIT_VIDEO_FILTER_QT4(ADM_vf_aiEnhance${QT_LIBRARY_EXTENSION} ${AiEnhanceQT_SRCS} ${AiEnhanceQT_HEADERS} ${AiEnhanceQT_UI} ${AiEnhanceCommon_SRCS} qt4/DIA_flyAiEnhance.cpp)
IF (DO_QT4)
    ADM_LINK_THREAD(ADM_vf_aiEnhance${QT_LIBRARY_EXTENSION} )
ENDIF (DO_QT4)
# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(AiEnhanceCli_SRCS cli/DIA_aiEnhance.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_aiEnhanceCli ${AiEnhanceCli_SRCS} ${AiEnhanceCommon_SRCS})
IF (DO_CLI)
    ADM_LINK_THREAD(ADM_vf_aiEnhanceCli)
ENDIF (DO_CLI)
#

