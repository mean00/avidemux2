INCLUDE(vf_plugin)

SET(resizeCommon_SRCS ADM_vidMPLResize.cpp)
SET(resizeCommonGtk_SRCS gtk/gtk_resize.cpp)
SET(resizeCommonQT_SRCS qt4/Q_resize.cpp)
SET(resizeCommonQT_Headers qt4/Q_resize.h)
SET(resizeCommonQT_UI qt4/resizing)

INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/")
#INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

INCLUDE(vf_plugin_gtk)
INIT_VIDEOFILTER_PLUGIN_GTK(ADM_vf_mplayerResize_gtk ${resizeCommonGtk_SRCS} ${resizeCommon_SRCS})

IF (GTK_FOUND AND GTHREAD_FOUND)
	TARGET_LINK_LIBRARIES(ADM_vf_mplayerResize_gtk ADM_libswscale ADM_libavcodec ADM_libavutil ADM_UIGtk)
ENDIF (GTK_FOUND AND GTHREAD_FOUND)

INCLUDE(vf_plugin_qt4)
INIT_VIDEOFILTER_PLUGIN_QT4(ADM_vf_mplayerResize_qt4 ${resizeCommonQT_SRCS} ${resizeCommonQT_Headers} ${resizeCommonQT_UI} ${resizeCommon_SRCS})

IF (QT4_FOUND)
	TARGET_LINK_LIBRARIES(ADM_vf_mplayerResize_qt4 ADM_libswscale ADM_libavcodec ADM_libavutil ADM_UIQT4)
ENDIF (QT4_FOUND)
