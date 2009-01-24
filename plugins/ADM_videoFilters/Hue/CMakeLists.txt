INCLUDE(vf_plugin)


SET(HueCommon_SRCS 
ADM_vidHue.cpp  DIA_flyHue.cpp
)

INCLUDE(vf_plugin_gtk)
INCLUDE(vf_plugin_qt4)

SET(HueGtk_SRCS gtk/DIA_hue.cpp)
SET(HueQT_SRCS  qt4/Q_hue.cpp)
SET(HueQT_HEADERS  qt4/Q_hue.h)
SET(HueQT_UI    qt4/hue)

INIT_VIDEOFILTER_PLUGIN_QT4(ADM_vf_hue_qt4 
                ${HueQT_SRCS} ${HueQT_HEADERS} ${HueQT_UI} ${HueCommon_SRCS})
INIT_VIDEOFILTER_PLUGIN_GTK(  ADM_vf_hue_gtk 
                ${HueGtk_SRCS} ${HueCommon_SRCS})

