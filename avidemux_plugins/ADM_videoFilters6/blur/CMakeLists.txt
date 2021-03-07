INCLUDE(vf_plugin)


SET(BlurCommon_SRCS 
ADM_vidBlur.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(BlurQT_SRCS  qt4/Q_blur.cpp)
SET(BlurQT_HEADERS  qt4/Q_blur.h)
SET(BlurQT_UI    qt4/blur)
INIT_VIDEO_FILTER_QT4(ADM_vf_blur${QT_LIBRARY_EXTENSION} ${BlurQT_SRCS} ${BlurQT_HEADERS} ${BlurQT_UI} ${BlurCommon_SRCS} qt4/DIA_flyBlur.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(BlurCli_SRCS cli/DIA_blur.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_blurCli ${BlurCli_SRCS} ${BlurCommon_SRCS})
#
