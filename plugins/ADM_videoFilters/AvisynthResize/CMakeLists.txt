INCLUDE(vf_plugin)

SET(avisynthResizeCommon_SRCS 
	ADM_resizebis.cpp  ADM_resizeter.cpp  ADM_vidResize.cpp
)


SET(resizeCommonGtk_SRCS gtk/gtk_resize.cpp)

SET(resizeQt_SRCS qt4/Q_resize.cpp)
SET(resizeQt_Headers qt4/Q_resize.h)
SET(resizeQt_UI qt4/resizing)

INCLUDE(vf_plugin_gtk)
INCLUDE(vf_plugin_qt4)

INIT_VIDEOFILTER_PLUGIN_QT4(ADM_vf_avisynthResize_qt4 
                ${resizeQt_SRCS} ${resizeQt_Headers} ${resizeQt_UI} ${avisynthResizeCommon_SRCS})
INIT_VIDEOFILTER_PLUGIN_GTK(ADM_vf_avisynthResize_gtk 
                ${resizeCommonGtk_SRCS} ${avisynthResizeCommon_SRCS})
