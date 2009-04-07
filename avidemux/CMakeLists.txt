########################################
# Macros
########################################
MACRO (ADD_ADMLIB_ALL_TARGETS)
	FOREACH (_libName ${ARGV})
		TARGET_LINK_LIBRARIES(avidemux2_cli ${_libName}_cli)

		IF (ADM_UI_GTK)
			TARGET_LINK_LIBRARIES(avidemux2_gtk ${_libName}_gtk)
		ENDIF (ADM_UI_GTK)

		IF (ADM_UI_QT4)
			TARGET_LINK_LIBRARIES(avidemux2_qt4 ${_libName}_qt4)
		ENDIF (ADM_UI_QT4)
	ENDFOREACH (_libName ${ARGV})
ENDMACRO (ADD_ADMLIB_ALL_TARGETS)


MACRO (ADD_LIB_ALL_TARGETS)
	TARGET_LINK_LIBRARIES(avidemux2_cli ${ARGV})

	IF (ADM_UI_GTK)
		TARGET_LINK_LIBRARIES(avidemux2_gtk ${ARGV})
	ENDIF (ADM_UI_GTK)

	IF (ADM_UI_QT4)
		TARGET_LINK_LIBRARIES(avidemux2_qt4 ${ARGV})
	ENDIF (ADM_UI_QT4)
ENDMACRO (ADD_LIB_ALL_TARGETS)


MACRO (ADD_LDFLAGS_ALL_TARGETS)
	ADD_TARGET_LDFLAGS(avidemux2_cli ${ARGV})

	IF (ADM_UI_GTK)
		ADD_TARGET_LDFLAGS(avidemux2_gtk ${ARGV})
	ENDIF (ADM_UI_GTK)

	IF (ADM_UI_QT4)
		ADD_TARGET_LDFLAGS(avidemux2_qt4 ${ARGV})
	ENDIF (ADM_UI_QT4)
ENDMACRO (ADD_LDFLAGS_ALL_TARGETS)

########################################
# Definitions and Includes
########################################
ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES)
ADD_DEFINITIONS(-DHAVE_CONFIG_H)

IF (ADM_DEBUG)
	ADD_DEFINITIONS(-DADM_DEBUG)
ENDIF (ADM_DEBUG)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/config")
# temp
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/ADM_audioFilter_old/")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/ADM_libraries/")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/ADM_libraries/ADM_ffmpeg/libavutil")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/ADM_libraries/ADM_ffmpeg/")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/ADM_libraries/ADM_ffmpeg/ADM_lavcodec")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/ADM_muxerGate/include/")

IF (GETTEXT_FOUND)
	INCLUDE_DIRECTORIES(${GETTEXT_INCLUDE_DIR})
ENDIF (GETTEXT_FOUND)

###########################################
#  Our own shared libs
###########################################
SET(ADM_CORES
        ADM_core
        ADM_coreUI
        ADM_coreImage
        ADM_audioParser
        ADM_coreAudioEncoder
        ADM_coreAudio
        ADM_audioFilter
        ADM_coreDemuxer
        ADM_coreDemuxerMpeg
        ADM_coreAudioDevice
        ADM_coreUtils
        ADM_coreMuxer
)
FOREACH(_current ${ADM_CORES})
        INCLUDE_DIRECTORIES(${_current}/include)
ENDFOREACH(_current ${ADM_CORES})


########################################
# Embed resources for executables
########################################
IF (APPLE)
	# add icon
	SET(ADM_EXE_SRCS ${ADM_EXE_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/xpm/avidemux.icns)
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/xpm/avidemux.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	# bundle version info
	SET(ADM_OSX_VERSION "${VERSION}")

	IF (Subversion_FOUND)
		SET(ADM_OSX_VERSION "${ADM_OSX_VERSION} (r${ADM_SUBVERSION})")
	ENDIF (Subversion_FOUND)

	SET(MACOSX_BUNDLE_ICON_FILE avidemux.icns)
	SET(MACOSX_BUNDLE_INFO_STRING "Avidemux ${ADM_OSX_VERSION}")
	SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${ADM_OSX_VERSION}") 
	SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${ADM_OSX_VERSION}")
	SET(MACOSX_BUNDLE_VERSION "${ADM_OSX_VERSION}")

	# include translation files in bundle
	# this isn't the cleanest approach but there's not much we can do until the po directory is under the avidemux directory
	FILE(GLOB PO_FILES ${CMAKE_SOURCE_DIR}/po/*.po)

	FOREACH(_file ${PO_FILES})
		GET_FILENAME_COMPONENT(_filename ${_file} NAME_WE)
		SET(_mofile "${CMAKE_BINARY_DIR}/po/${_filename}/avidemux.mo")

		SET(MO_FILES ${MO_FILES} ${_mofile})
		SET_SOURCE_FILES_PROPERTIES(${_mofile} PROPERTIES GENERATED TRUE MACOSX_PACKAGE_LOCATION "Resources/locale/${_filename}/LC_MESSAGES")
	ENDFOREACH(_file)

	FILE(GLOB TS_FILES ${CMAKE_SOURCE_DIR}/po/*.ts)

	FOREACH(_file ${TS_FILES})
		GET_FILENAME_COMPONENT(_filename ${_file} NAME_WE)
		SET(_qmfile "${CMAKE_BINARY_DIR}/po/${_filename}.qm")

		SET(QM_FILES ${QM_FILES} ${_qmfile})
		SET_SOURCE_FILES_PROPERTIES(${_qmfile} PROPERTIES GENERATED TRUE MACOSX_PACKAGE_LOCATION "Resources/locale")
	ENDFOREACH(_file)
ENDIF (APPLE)

IF (WIN32)
	# add icon and version info
	SET(FILEVERSION_STRING "${VERSION}.${ADM_SUBVERSION}")
	SET(PRODUCTVERSION_STRING "${VERSION}")
	STRING(REPLACE "." "," FILEVERSION ${FILEVERSION_STRING})
	STRING(REPLACE "." "," PRODUCTVERSION ${PRODUCTVERSION_STRING})
	SET(PRODUCTVERSION "${PRODUCTVERSION},0")

	#CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/admWin32.rc.in" ${CMAKE_CURRENT_BINARY_DIR}/admWin32.rc IMMEDIATE)
	#ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/admWin32.obj COMMAND windres.exe -i ${CMAKE_CURRENT_BINARY_DIR}/admWin32.rc -o ${CMAKE_CURRENT_BINARY_DIR}/admWin32.obj -O coff --define VS_VERSION_INFO=1)

	#SET(ADM_EXE_SRCS ${ADM_EXE_SRCS} "${CMAKE_CURRENT_BINARY_DIR}/admWin32.obj")
ENDIF (WIN32)

########################################
# Our exectuables
########################################
SET(ADM_EXE_SRCS ${ADM_EXE_SRCS} ADM_preview.cpp  
gtk_gui.cpp  
gui_autodrive.cpp  
GUI_jobs.cpp  
gui_navigate.cpp  
gui_play.cpp  
gui_save.cpp  
gui_savenew.cpp  
gui_misc.cpp
main.cpp  
gui_action.cpp
gui_blackframes.cpp
ADM_imageLoader.cpp  
ADM_gettext.cpp
)

ADD_EXECUTABLE(avidemux2_cli MACOSX_BUNDLE ${ADM_EXE_SRCS} ${MO_FILES})
ADD_TARGET_CFLAGS(avidemux2_cli "-I${CMAKE_BINARY_DIR}/config/cli")
ADD_DEPENDENCIES(avidemux2_cli mofiles)

IF (ADM_UI_GTK)
	ADD_EXECUTABLE(avidemux2_gtk MACOSX_BUNDLE ${ADM_EXE_SRCS} ${MO_FILES})
	ADD_TARGET_CFLAGS(avidemux2_gtk "-I${CMAKE_BINARY_DIR}/config/gtk")
	ADD_DEPENDENCIES(avidemux2_gtk mofiles)
ENDIF (ADM_UI_GTK)

IF (ADM_UI_QT4)
	ADD_EXECUTABLE(avidemux2_qt4 MACOSX_BUNDLE ${ADM_EXE_SRCS} ${QM_FILES})
	ADD_TARGET_CFLAGS(avidemux2_qt4 "-I${CMAKE_BINARY_DIR}/config/qt4")
	ADD_DEPENDENCIES(avidemux2_qt4 qmfiles)
ENDIF (ADM_UI_QT4)

SDLify(main.cpp)
ADD_SOURCE_CFLAGS(main.cpp "-DADM_SUBVERSION=${ADM_SUBVERSION}")

########################################
# Compile and link general subdirs
########################################
SET(ADM_SUBDIR 
	ADM_audioFilter_old
	ADM_editor
	ADM_muxerGate
	ADM_audiocodec
	#ADM_audio
	ADM_script
	ADM_filter
	ADM_videoFilter
	ADM_video
)
FOREACH(_current ${ADM_SUBDIR})
	ADD_SUBDIRECTORY(${_current})
	ADD_ADMLIB_ALL_TARGETS(${_current})
ENDFOREACH(_current ${ADM_SUBDIR})

###########################################
# Construct ADM_UIs
###########################################
ADD_SUBDIRECTORY(ADM_UIs)


#########################################
# Tails
#########################################
SET(ADM_TAILS
	ADM_osSupport
	ADM_encoder
	ADM_toolkit
	ADM_codecs
	ADM_requant
	)

FOREACH(_current ${ADM_TAILS})
	ADD_SUBDIRECTORY(${_current})
	ADD_ADMLIB_ALL_TARGETS(${_current})
ENDFOREACH(_current ${ADM_TAILS})
###########################################
# Construct ADM_libraries
###########################################
ADD_SUBDIRECTORY(ADM_libraries)

ADD_ADMLIB_ALL_TARGETS(
	ADM_libmpeg2enc
	ADM_xvidratectl
)
ADD_LIB_ALL_TARGETS(
	ADM_smjs
	ADM_libavutil
	ADM_libpostproc
	ADM_libswscale
	ADM_libavcodec
	ADM_libavformat)


IF (ADM_DEBUG AND FIND_LEAKS)
	ADD_ADMLIB_ALL_TARGETS(ADM_nvwa)
ENDIF (ADM_DEBUG AND FIND_LEAKS)

IF (APPLE AND USE_SDL)
	IF (ADM_UI_GTK)
		TARGET_LINK_LIBRARIES(avidemux2_gtk ADM_libsdl_gtk)
	ENDIF (ADM_UI_GTK)

	IF (ADM_UI_QT4)
		TARGET_LINK_LIBRARIES(avidemux2_qt4 ADM_libsdl_qt4)
	ENDIF (ADM_UI_QT4)
ENDIF (APPLE AND USE_SDL)

###########################################
# Construct ADM_userInterface
###########################################
ADD_SUBDIRECTORY(ADM_userInterfaces)

TARGET_LINK_LIBRARIES(avidemux2_cli
	ADM_UI_None_cli
	ADM_commonUI_cli
	#ADM_ocr_cli
	ADM_dialogNone_cli
	ADM_dialogFiltersNone_cli
	ADM_dialogGuiNone_cli
        ADM_UICli
        ADM_render_cli
)

IF (ADM_UI_GTK)
	TARGET_LINK_LIBRARIES(avidemux2_gtk
		ADM_UI_GTK_gtk
		ADM_commonUI_gtk
		#ADM_ocr_gtk
		#ADM_ocrGtk_gtk
		ADM_dialogGtk_gtk
		ADM_filtersGtk_gtk
		ADM_gui2Gtk_gtk
		ADM_commonUI_gtk
                ADM_toolkitGtk_gtk
                ADM_UIGtk
                ADM_render_gtk
                )
ENDIF (ADM_UI_GTK)

IF (ADM_UI_QT4)
	TARGET_LINK_LIBRARIES(avidemux2_qt4
		ADM_UI_QT4_qt4
		ADM_commonUI_qt4
		#ADM_ocr_qt4
		#ADM_ocrQt4_qt4
		ADM_dialogQt4_qt4
		ADM_guiQt4_qt4
		ADM_filtersQt4_qt4
		ADM_commonUI_qt4
		ADM_dialogQt4_qt4
                ADM_UIQT4
                ADM_render_qt4
)
ENDIF (ADM_UI_QT4)
# Add the core libs
FOREACH(_current ${ADM_CORES})
	ADD_SUBDIRECTORY(${_current})
	ADD_LIB_ALL_TARGETS(${_current})
ENDFOREACH(_current ${ADM_CORES})

# To be removed
#ADD_SUBDIRECTORY(ADM_audioEncoder)

###########################################
# External libs
###########################################
# gettext
IF (GETTEXT_FOUND)
	TARGET_LINK_LIBRARIES(avidemux2_cli ${GETTEXT_LIBRARY_DIR})

	IF (ADM_UI_GTK)
		TARGET_LINK_LIBRARIES(avidemux2_gtk ${GETTEXT_LIBRARY_DIR})
	ENDIF (ADM_UI_GTK)
ENDIF (GETTEXT_FOUND)

# pthreads
IF (UNIX AND PTHREAD_FOUND)
	ADD_LIB_ALL_TARGETS("${PTHREAD_LIBRARIES}")
ENDIF (UNIX AND PTHREAD_FOUND)

# PNG
IF (USE_PNG)
	TARGET_LINK_LIBRARIES(avidemux2_cli ${PNG_LIBRARIES})
	
	IF (ADM_UI_GTK)
		TARGET_LINK_LIBRARIES(avidemux2_gtk ${PNG_LIBRARIES})
	ENDIF (ADM_UI_GTK)
	
	IF (ADM_UI_QT4)
		IF (APPLE AND NOT QT_USE_FRAMEWORKS)
			TARGET_LINK_LIBRARIES(avidemux2_qt4 ${PNG_LIBRARIES})
		ELSEIF (NOT APPLE)
			TARGET_LINK_LIBRARIES(avidemux2_qt4 ${PNG_LIBRARIES})
		ENDIF (APPLE AND NOT QT_USE_FRAMEWORKS)
	ENDIF (ADM_UI_QT4)
ENDIF (USE_PNG)


# XVideo
IF (USE_XV)
	IF (ADM_UI_GTK)
		TARGET_LINK_LIBRARIES(avidemux2_gtk ${XVIDEO_LIBRARY_DIR})
	ENDIF (ADM_UI_GTK)

	IF (ADM_UI_QT4)
		TARGET_LINK_LIBRARIES(avidemux2_qt4 ${XVIDEO_LIBRARY_DIR})
	ENDIF (ADM_UI_QT4)
ENDIF (USE_XV)

# SDL
IF (USE_SDL)
	IF (ADM_UI_GTK)
		TARGET_LINK_LIBRARIES(avidemux2_gtk ${SDL_LIBRARY})
	ENDIF (ADM_UI_GTK)

	IF (ADM_UI_QT4)
		TARGET_LINK_LIBRARIES(avidemux2_qt4 ${SDL_LIBRARY})
	ENDIF (ADM_UI_QT4)
ENDIF (USE_SDL)
# LAME
IF (HAVE_LIBMP3LAME)
	ADD_LIB_ALL_TARGETS("${LAME_LIBRARY_DIR}")
ENDIF (HAVE_LIBMP3LAME)

# Vorbis
IF (USE_VORBIS)
	ADD_LIB_ALL_TARGETS("${VORBIS_LIBRARY_DIR}")
	ADD_LIB_ALL_TARGETS("${VORBISENC_LIBRARY_DIR}")
ENDIF (USE_VORBIS)

# FAAC
IF (USE_FAAC)
	ADD_LIB_ALL_TARGETS("${FAAC_LIBRARY_DIR}")
ENDIF (USE_FAAC)

# Aften
IF (USE_AFTEN)
	ADD_LIB_ALL_TARGETS("${AFTEN_LIBRARY_DIR}")
ENDIF (USE_AFTEN)

# Secret Rabbit Code
IF (USE_SRC)
	ADD_LIB_ALL_TARGETS("${SRC_LIBRARY_DIR}")
ENDIF (USE_SRC)

# Xvid
IF (USE_XVID_4)
	ADD_LIB_ALL_TARGETS("${XVID_LIBRARY_DIR}")
ENDIF (USE_XVID_4)

###########################################
# UI Specific
###########################################
IF (ADM_UI_GTK)
	ADD_TARGET_LDFLAGS(avidemux2_gtk ${GTK_LDFLAGS} ${GTHREAD_LDFLAGS})
ENDIF (ADM_UI_GTK)

IF (ADM_UI_QT4)
	TARGET_LINK_LIBRARIES(avidemux2_qt4 ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
ENDIF (ADM_UI_QT4)

###########################################
# OS Specific
###########################################
IF (WIN32)
	ADD_ADMLIB_ALL_TARGETS(ADM_osSupport)

	IF (MINGW)
		ADD_LIB_ALL_TARGETS(-mno-cygwin)
	ENDIF (MINGW)

	ADD_LIB_ALL_TARGETS(winmm)

	ADD_TARGET_LDFLAGS(avidemux2_cli "-Wl,-subsystem,console")

	IF (ADM_UI_GTK)
		ADD_TARGET_LDFLAGS(avidemux2_gtk "-mwindows")
	ENDIF (ADM_UI_GTK)

	IF (ADM_UI_QT4)
		ADD_TARGET_LDFLAGS(avidemux2_qt4 "-mwindows")
	ENDIF (ADM_UI_QT4)
ENDIF (WIN32)

IF (APPLE)
	ADD_LIB_ALL_TARGETS("-framework CoreServices -framework CoreAudio -framework AudioUnit -framework Carbon -framework Cocoa")

	# for Leopard but it doesn't hurt Tiger
	ADD_LIB_ALL_TARGETS("-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDIF (APPLE)

###########################################
# Install
###########################################
SET(ADM_EXES avidemux2_cli)

IF (ADM_UI_GTK)
	SET(ADM_EXES ${ADM_EXES} avidemux2_gtk)
ENDIF (ADM_UI_GTK)

IF (ADM_UI_QT4)
	SET(ADM_EXES ${ADM_EXES} avidemux2_qt4)
ENDIF (ADM_UI_QT4)

INSTALL(TARGETS ${ADM_EXES} RUNTIME DESTINATION ${BIN_DIR} LIBRARY DESTINATION lib)
