MACRO(AS_NEEDED name)
        TARGET_LINK_LIBRARIES( ${name} "-Wl,--as-needed")
ENDMACRO(AS_NEEDED name)

