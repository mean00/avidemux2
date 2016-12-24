#
INCLUDE(vf_plugin)

MACRO(addDeps tgt)
        TARGET_LINK_LIBRARIES(${tgt} ADM_libswscale)
        TARGET_LINK_LIBRARIES(${tgt} ADM_libavcodec)
        TARGET_LINK_LIBRARIES(${tgt} ADM_libavutil)
ENDMACRO(addDeps tgt)

SET(mpdelogoCommon_SRCS 
ADM_vidMPdelogo.cpp
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(mpdelogoQT_SRCS  qt4/Q_mpdelogo.cpp )
SET(mpdelogoQT_HEADERS  qt4/Q_mpdelogo.h)
SET(mpdelogoQT_UI    qt4/mpdelogo)
IF(DO_QT4)
  INIT_VIDEO_FILTER_QT4(ADM_vf_mpdelogo${QT_LIBRARY_EXTENSION} ${mpdelogoQT_SRCS} ${mpdelogoQT_HEADERS} ${mpdelogoQT_UI} ${mpdelogoCommon_SRCS} qt4/DIA_flyMpDelogo.cpp)
  addDeps(ADM_vf_mpdelogo${QT_LIBRARY_EXTENSION})
ENDIF(DO_QT4)
# /QT4


#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(mpdelogoCli_SRCS cli/DIA_mpdelogo.cpp)
IF(DO_CLI)
  INIT_VIDEO_FILTER_CLI(  ADM_vf_mpdelogoCli ${mpdelogoCli_SRCS} ${mpdelogoCommon_SRCS})
  addDeps(ADM_vf_mpdelogoCli)
ENDIF(DO_CLI)
##%%%%%%%%%%%%%%%%%%

