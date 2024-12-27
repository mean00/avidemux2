#
#
#
MACRO(ADM_PREBUILD)
  LIST(APPEND PlatformLibs   "-lm -lstdc++")
  LIST(APPEND PlatformLibs  "-framework CoreServices -framework CoreAudio -framework AudioUnit -framework Carbon -framework Accelerate")
  LIST(APPEND PlatformLibs  "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDMACRO(ADM_PREBUILD)
#
