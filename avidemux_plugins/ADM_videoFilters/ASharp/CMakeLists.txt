INCLUDE(vf_plugin)


SET(asharpCommon_SRCS 
ADM_vidAsharp.cpp  DIA_flyAsharp.cpp
)

INCLUDE(vf_plugin_gtk)
INCLUDE(vf_plugin_qt4)

SET(asharpGtk_SRCS gtk/DIA_asharp.cpp)
SET(asharpQT_SRCS  qt4/Q_asharp.cpp)
SET(asharpQT_HEADERS  qt4/Q_asharp.h)
SET(asharpQT_UI    qt4/asharp)

INIT_VIDEOFILTER_PLUGIN_QT4(ADM_vf_asharp_qt4 
                ${asharpQT_SRCS} ${asharpQT_HEADERS} ${asharpQT_UI} ${asharpCommon_SRCS})
INIT_VIDEOFILTER_PLUGIN_GTK(  ADM_vf_asharp_gtk 
                ${asharpGtk_SRCS} ${asharpCommon_SRCS})

