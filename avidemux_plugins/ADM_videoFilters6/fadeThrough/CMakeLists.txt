INCLUDE(vf_plugin)

SET(FadeThroughCommon_SRCS ADM_vidFadeThrough.cpp)
IF(DO_QT4)
    include(admCheckThreads)
ENDIF(DO_QT4)
IF(DO_CLI)
    include(admCheckThreads)
ENDIF(DO_CLI)

# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(FadeThroughQT_SRCS  qt4/Q_fadeThrough.cpp)
SET(FadeThroughQT_HEADERS  qt4/Q_fadeThrough.h)
SET(FadeThroughQT_UI    qt4/fadeThrough)
INIT_VIDEO_FILTER_QT4(ADM_vf_fadeThrough${QT_LIBRARY_EXTENSION} ${FadeThroughQT_SRCS} ${FadeThroughQT_HEADERS} ${FadeThroughQT_UI} ${FadeThroughCommon_SRCS} qt4/DIA_flyFadeThrough.cpp qt4/fadeThrough.qrc)
IF (DO_QT4)
    ADM_LINK_THREAD(ADM_vf_fadeThrough${QT_LIBRARY_EXTENSION} )
ENDIF (DO_QT4)
# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(FadeThroughCli_SRCS cli/DIA_fadeThrough.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_fadeThroughCli ${FadeThroughCli_SRCS} ${FadeThroughCommon_SRCS})
IF (DO_CLI)
    ADM_LINK_THREAD(ADM_vf_fadeThroughCli)
ENDIF (DO_CLI)
#


