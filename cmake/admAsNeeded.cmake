MACRO(AS_NEEDED name)
	IF(NOT APPLE)
        	TARGET_LINK_LIBRARIES( ${name} "-Wl,--as-needed")
	ENDIF(NOT APPLE)
ENDMACRO(AS_NEEDED name)

MACRO(ADM_ADD_SHARED_LIBRARY name )
        ADD_LIBRARY(${name} SHARED ${ARGN})
	AS_NEEDED(${name})
        if(UNIX AND NOT APPLE)
        	TARGET_LINK_LIBRARIES( ${name} "-Wl,-z,defs")
        	TARGET_LINK_LIBRARIES( ${name} "stdc++") # for clang
        endif(UNIX AND NOT APPLE)

ENDMACRO(ADM_ADD_SHARED_LIBRARY name )

MACRO(ADM_TARGET_NO_EXCEPTION  name)
		ADD_TARGET_CFLAGS(${name} "-fno-exceptions -fno-rtti")
ENDMACRO(ADM_TARGET_NO_EXCEPTION  name)
MACRO(ADM_NO_EXCEPTION  )
		ADD_DEFINITIONS( "-fno-exceptions -fno-rtti")
ENDMACRO(ADM_NO_EXCEPTION  )
