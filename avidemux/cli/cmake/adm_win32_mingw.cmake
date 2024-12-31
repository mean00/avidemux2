MACRO(ADM_PREBUILD)
  LIST(APPEND PlatformLibs   "-lm -lstdc++")
  LIST(APPEND PlatformLibs   "winmm -mwindows -Wl,--export-all-symbols")
ENDMACRO()
#

