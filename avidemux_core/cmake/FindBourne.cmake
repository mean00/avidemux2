IF(NOT BASH_EXECUTABLE)
	MESSAGE(STATUS "Checking for Bourne shell")
	MESSAGE(STATUS "*************************")

	find_program(BASH_EXECUTABLE
		bash
		${BASH_DIR}
		/bin
		/usr/bin 
		/usr/local/bin
		/sbin)
	SET(BASH_EXECUTABLE ${BASH_EXECUTABLE} CACHE STRING "")

	IF(BASH_EXECUTABLE)
		MESSAGE(STATUS "Found Bourne shell")
		
		IF(VERBOSE)
			MESSAGE(STATUS "Path: ${BASH_EXECUTABLE}")
		ENDIF()
	ELSE()
		MESSAGE(FATAL_ERROR "Bourne shell not found")
	ENDIF()

	MESSAGE("")
ENDIF()