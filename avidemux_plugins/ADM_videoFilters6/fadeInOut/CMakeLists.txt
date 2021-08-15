INCLUDE(vf_plugin)

#SET(FadeInOutCommon_SRCS ADM_vidFadeInOut.cpp)

# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(FadeInOutQT_SRCS  qt4/Q_fadeInOut.cpp)
SET(FadeInOutQT_HEADERS  qt4/Q_fadeInOut.h)
SET(FadeInOutQT_UI    qt4/fadeInOut)
INIT_VIDEO_FILTER_QT4(ADM_vf_fadeIn${QT_LIBRARY_EXTENSION} ${FadeInOutQT_SRCS} ${FadeInOutQT_HEADERS} ${FadeInOutQT_UI} ADM_vidFadeIn.cpp qt4/DIA_flyFadeInOut.cpp qt4/fadeInOut.qrc)
INIT_VIDEO_FILTER_QT4(ADM_vf_fadeOut${QT_LIBRARY_EXTENSION} ${FadeInOutQT_SRCS} ${FadeInOutQT_HEADERS} ${FadeInOutQT_UI} ADM_vidFadeOut.cpp qt4/DIA_flyFadeInOut.cpp qt4/fadeInOut.qrc)
# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(FadeInOutCli_SRCS cli/DIA_fadeInOut.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_fadeInCli ${FadeInOutCli_SRCS} ADM_vidFadeIn.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_fadeOutCli ${FadeInOutCli_SRCS} ADM_vidFadeOut.cpp)

#


