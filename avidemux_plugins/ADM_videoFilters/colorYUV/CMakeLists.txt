INCLUDE(vf_plugin)


SET(colorYUVCommon_SRCS 
ADM_vidColorYuv.cpp
)

INCLUDE(vf_plugin_gtk)

SET(colorYUVGtk_SRCS gtk/DIA_coloryuv.cpp)

INIT_VIDEOFILTER_PLUGIN_GTK(  ADM_vf_colorYUV_gtk 
                ${colorYUVGtk_SRCS} ${colorYUVCommon_SRCS})

