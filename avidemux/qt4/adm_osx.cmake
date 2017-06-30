#
SET(bundle avidemux3_${QT_EXTENSION})
#
MACRO(ADM_PREBUILD)
        LIST(APPEND PlatformLibs   "-lm -lstdc++")
        LIST(APPEND PlatformLibs  "-framework CoreServices -framework CoreAudio -framework AudioUnit -framework Carbon")
        LIST(APPEND PlatformLibs  "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDMACRO(ADM_PREBUILD)
#
MACRO(ADM_MAIN_APP)
    ADD_EXECUTABLE(avidemux3_${QT_EXTENSION}  MACOSX_BUNDLE ${ADM_EXE_SRCS})
    set_property(TARGET avidemux3_${QT_EXTENSION} PROPERTY OUTPUT_NAME Avidemux${AVIDEMUX_MAJOR_MINOR})
    qt5_use_modules( avidemux3_${QT_EXTENSION}   Widgets )
	IF (USE_SDL)
		TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION}   ADM_libsdl)
	ENDIF(USE_SDL)
ENDMACRO(ADM_MAIN_APP)
#
MACRO(ADM_POSTBUILD)
    SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/..)
    INSTALL(TARGETS avidemux3_${QT_EXTENSION}  BUNDLE DESTINATION . COMPONENT RUNTIME)
    #include(DeployQt5)
    #install_qt5_executable(Avidemux${AVIDEMUX_MAJOR_MINOR}.app) #  "qcocoa")
ENDMACRO(ADM_POSTBUILD)

