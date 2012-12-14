INCLUDE(vf_plugin)

MACRO(addDeps tgt)
        TARGET_LINK_LIBRARIES(${tgt} ADM_libswscale)
        TARGET_LINK_LIBRARIES(${tgt} ADM_libavcodec)
        TARGET_LINK_LIBRARIES(${tgt} ADM_libavutil)
ENDMACRO(addDeps tgt)

SET(mpdelogoCommon_SRCS 
ADM_vidMPdelogo.cpp
DIA_flyMpDelogo.cpp
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(mpdelogoQT_SRCS  qt4/Q_mpdelogo.cpp)
SET(mpdelogoQT_HEADERS  qt4/Q_mpdelogo.h)
SET(mpdelogoQT_UI    qt4/mpdelogo)
IF(DO_QT4)
  INIT_VIDEO_FILTER_QT4(ADM_vf_mpdelogoQt4 ${mpdelogoQT_SRCS} ${mpdelogoQT_HEADERS} ${mpdelogoQT_UI} ${mpdelogoCommon_SRCS})
  addDeps(ADM_vf_mpdelogoQt4)
ENDIF(DO_QT4)
# /QT4


#------------- Gtk Version ---------------
#INCLUDE(vf_plugin_gtk)
#SET(mpdelogoGtk_SRCS gtk/DIA_mpdelogo.cpp)
#INIT_VIDEO_FILTER_GTK(ADM_vf_mpdelogoGtk ${mpdelogoGtk_SRCS} ${mpdelogoCommon_SRCS})

#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(mpdelogoCli_SRCS cli/DIA_mpdelogo.cpp)
IF(DO_CLI)
  INIT_VIDEO_FILTER_CLI(  ADM_vf_mpdelogoCli ${mpdelogoCli_SRCS} ${mpdelogoCommon_SRCS})
  addDeps(ADM_vf_mpdelogoCli)
ENDIF(DO_CLI)
##%%%%%%%%%%%%%%%%%%

