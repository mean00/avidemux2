#
MACRO(ADM_PREBUILD)
        LIST(APPEND PlatformLibs   "-lm -lstdc++")
        LIST(APPEND PlatformLibs  "-framework CoreServices -framework CoreAudio -framework AudioUnit -framework Carbon")
        LIST(APPEND PlatformLibs  "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDMACRO(ADM_PREBUILD)
#
MACRO(ADM_MAIN_APP)
        ADD_EXECUTABLE(avidemux3_${QT_EXTENSION} MACOSX_BUNDLE ${ADM_EXE_SRCS})
        set_property(TARGET avidemux3_${QT_EXTENSION} PROPERTY OUTPUT_NAME avidemux)
	IF (USE_SDL)
		TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ADM_libsdl)
	ENDIF(USE_SDL)
ENDMACRO(ADM_MAIN_APP)
#
MACRO(ADM_POSTBUILD)
	#ADM_INSTALL_BIN(avidemux3_${QT_EXTENSION})
ENDMACRO(ADM_POSTBUILD)

