INCLUDE(vf_plugin)


SET(contrastCommon_SRCS 
ADM_vidContrast.cpp  DIA_flyContrast.cpp
)

INCLUDE(vf_plugin_gtk)
INCLUDE(vf_plugin_qt4)

SET(contrastGtk_SRCS gtk/DIA_contrast.cpp)
SET(contrastQT_SRCS  qt4/Q_contrast.cpp)
SET(contrastQT_HEADERS  qt4/Q_contrast.h)
SET(contrastQT_UI    qt4/contrast)

INIT_VIDEOFILTER_PLUGIN_QT4(ADM_vf_contrast_qt4 
                ${contrastQT_SRCS} ${contrastQT_HEADERS} ${contrastQT_UI} ${contrastCommon_SRCS})
INIT_VIDEOFILTER_PLUGIN_GTK(  ADM_vf_contrast_gtk 
                ${contrastGtk_SRCS} ${contrastCommon_SRCS})

