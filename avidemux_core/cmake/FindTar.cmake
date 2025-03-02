IF(NOT TAR_EXECUTABLE)
	MESSAGE(STATUS "Checking for tar")
	MESSAGE(STATUS "****************")

	find_program(TAR_EXECUTABLE tar)
	SET(TAR_EXECUTABLE ${TAR_EXECUTABLE} CACHE STRING "")

	IF(TAR_EXECUTABLE)
		MESSAGE(STATUS "Found tar")

		IF(VERBOSE)
			MESSAGE(STATUS "Path: ${TAR_EXECUTABLE}")
		ENDIF()
	ELSE()
		MESSAGE(FATAL_ERROR "tar not found")
	ENDIF()

	MESSAGE("")
ENDIF()