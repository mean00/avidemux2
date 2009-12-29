# ARGV2 - capable flag
MACRO(ADM_DISPLAY _name)
	SET(PROCEED 1)

	IF (NOT ${ARGV2})
		SET(PROCEED 0)
	ENDIF (NOT ${ARGV2})

	IF (${PROCEED})
		IF (${ARGV1})
			SET(s "Yes")
		ELSE (${ARGV1})
			SET(s "No")
		ENDIF (${ARGV1})

		MESSAGE("   ${_name}  ${s}")
	ENDIF (${PROCEED})
ENDMACRO(ADM_DISPLAY)

