INCLUDE(vf_plugin)


SET(equalizerCommon_SRCS 
ADM_vidEqualizer.cpp
)

INCLUDE(vf_plugin_gtk)

SET(equalizerGtk_SRCS gtk/DIA_equalizer.cpp)

INIT_VIDEOFILTER_PLUGIN_GTK(  ADM_vf_equalizer_gtk 
                ${equalizerGtk_SRCS} ${equalizerCommon_SRCS})

