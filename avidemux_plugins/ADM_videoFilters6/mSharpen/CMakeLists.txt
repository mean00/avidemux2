
SET(msharpenCommon_SRCS 
ADM_vidMSharpen.cpp
)
INCLUDE(vf_plugin)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(msharpenQT_SRCS  qt4/Q_msharpen.cpp )
SET(msharpenQT_HEADERS  qt4/Q_msharpen.h)
SET(msharpenQT_UI    qt4/msharpen)
IF(DO_QT4)
  INIT_VIDEO_FILTER_QT4(ADM_vf_msharpen${QT_LIBRARY_EXTENSION} ${msharpenQT_SRCS} ${msharpenQT_HEADERS} ${msharpenQT_UI} ${msharpenCommon_SRCS} qt4/DIA_flymsharpen.cpp ${resource})
ENDIF(DO_QT4)
# /QT4


#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(msharpenCli_SRCS cli/DIA_flymsharpen.cpp)
IF(DO_CLI)
  INIT_VIDEO_FILTER_CLI(  ADM_vf_msharpenCli ${msharpenCli_SRCS} ${msharpenCommon_SRCS})
ENDIF(DO_CLI)
#
