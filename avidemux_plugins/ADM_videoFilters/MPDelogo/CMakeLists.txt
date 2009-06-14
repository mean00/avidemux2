INCLUDE(vf_plugin)


SET(mpdelogoCommon_SRCS 
ADM_vidMPdelogo.cpp
)

INCLUDE(vf_plugin_gtk)

SET(mpdelogoGtk_SRCS gtk/DIA_mpdelogo.cpp )
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/")

INIT_VIDEOFILTER_PLUGIN_GTK(  ADM_vf_mpdelogo_gtk 
                ${mpdelogoGtk_SRCS} ${mpdelogoCommon_SRCS})

IF (GTK_FOUND AND GTHREAD_FOUND)
        TARGET_LINK_LIBRARIES(ADM_vf_mpdelogo_gtk ADM_libswscale)
        TARGET_LINK_LIBRARIES(ADM_vf_mpdelogo_gtk ADM_libavcodec)
        TARGET_LINK_LIBRARIES(ADM_vf_mpdelogo_gtk ADM_libavutil)
ENDIF (GTK_FOUND AND GTHREAD_FOUND)        
