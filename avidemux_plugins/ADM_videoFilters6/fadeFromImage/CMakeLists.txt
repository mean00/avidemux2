INCLUDE(vf_plugin)

SET(FadeFromImageCommon_SRCS ADM_vidFadeFromImage.cpp)

# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(FadeFromImageQT_SRCS  qt4/Q_fadeFromImage.cpp)
SET(FadeFromImageQT_HEADERS  qt4/Q_fadeFromImage.h)
SET(FadeFromImageQT_UI    qt4/fadeFromImage)
INIT_VIDEO_FILTER_QT4(ADM_vf_fadeFromImage${QT_LIBRARY_EXTENSION} ${FadeFromImageQT_SRCS} ${FadeFromImageQT_HEADERS} ${FadeFromImageQT_UI} ${FadeFromImageCommon_SRCS} qt4/DIA_flyFadeFromImage.cpp)
# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(FadeFromImageCli_SRCS cli/DIA_fadeFromImage.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_fadeFromImageCli ${FadeFromImageCli_SRCS} ${FadeFromImageCommon_SRCS})
#


