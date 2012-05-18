INCLUDE(vf_plugin)


SET(HueCommon_SRCS 
ADM_vidHue.cpp  DIA_flyHue.cpp
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(HueQT_SRCS  qt4/Q_hue.cpp)
SET(HueQT_HEADERS  qt4/Q_hue.h)
SET(HueQT_UI    qt4/hue)
INIT_VIDEO_FILTER_QT4(ADM_vf_HueQt4 ${HueQT_SRCS} ${HueQT_HEADERS} ${HueQT_UI} ${HueCommon_SRCS})
# /QT4


#------------- Gtk Version ---------------
INCLUDE(vf_plugin_gtk)
SET(HueGtk_SRCS gtk/DIA_hue.cpp)
INIT_VIDEO_FILTER_GTK(ADM_vf_HueGtk ${HueGtk_SRCS} ${HueCommon_SRCS})

#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(HueCli_SRCS cli/DIA_hue.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_HueCli ${HueCli_SRCS} ${HueCommon_SRCS})
#
