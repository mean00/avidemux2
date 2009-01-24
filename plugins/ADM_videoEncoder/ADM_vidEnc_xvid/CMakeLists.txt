INCLUDE(admCheckXvid)
INCLUDE(admPluginVideoEncoder)

checkXvid()

IF (XVID_FOUND)
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

	SET(QT_PLUGIN_NAME "ADM_vidEnc_xvid_Qt")
	SET(GTK_PLUGIN_NAME "ADM_vidEnc_xvid_Gtk")
	SET(PLUGIN_SUBDIR "xvid")
	SET(ADM_vidEnc_xvid_SRCS interface.c  encoder.cpp  options.cpp  configGuiLoader.cpp)

	CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/config.h")

	ADD_SUBDIRECTORY(gtk)
	ADD_SUBDIRECTORY(qt4)
	ADD_LIBRARY(ADM_vidEnc_xvid SHARED ${ADM_vidEnc_xvid_SRCS})
	TARGET_LINK_LIBRARIES(ADM_vidEnc_xvid ADM_core)

	INIT_VIDEO_ENCODER_PLUGIN(ADM_vidEnc_xvid)
	INSTALL_VIDEO_ENCODER(ADM_vidEnc_xvid)
ENDIF (XVID_FOUND)