INCLUDE(admCheckQt4)
INCLUDE(admPluginVideoEncoder)

checkQt4()

IF (QT4_FOUND)
	QT4_WRAP_UI(x264Qt_headers
		x264ConfigDialog.ui x264CustomMatrixDialog.ui)
	QT4_WRAP_CPP(x264Qt_source
		x264ConfigDialog.h x264CustomMatrixDialog.h x264ZoneTableModel.h x264ZoneTableDelegate.h)
	SET(ADM_vidEnc_x264_Qt_SRCS 
		${x264Qt_headers} ${x264Qt_source} x264ConfigDialog.cpp x264CustomMatrixDialog.cpp x264ZoneTableModel.cpp x264ZoneTableDelegate.cpp)
	
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDE_DIR})
	ADD_LIBRARY(${QT_PLUGIN_NAME} SHARED ${ADM_vidEnc_x264_Qt_SRCS})
	TARGET_LINK_LIBRARIES(${QT_PLUGIN_NAME} ADM_vidEnc_x264 ADM_core ADM_coreUI ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

	INIT_VIDEO_ENCODER_PLUGIN(ADM_vidEnc_x264_Qt)
	INSTALL(TARGETS ADM_vidEnc_x264_Qt DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ADM_plugins/videoEncoder/${PLUGIN_SUBDIR}")
ENDIF (QT4_FOUND)