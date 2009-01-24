INCLUDE(vf_plugin)


SET(cnr2Common_SRCS 
ADM_vidCNR2.cpp
)

INCLUDE(vf_plugin_gtk)

SET(cnr2Gtk_SRCS gtk/DIA_cnr2.cpp)

INIT_VIDEOFILTER_PLUGIN_GTK(  ADM_vf_cnr2_gtk 
                ${cnr2Gtk_SRCS} ${cnr2Common_SRCS})

