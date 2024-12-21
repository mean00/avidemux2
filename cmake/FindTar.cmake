if (NOT TAR_EXECUTABLE)
	message(STATUS "Checking for tar")
	message(STATUS "****************")

	find_program(TAR_EXECUTABLE tar)
	set(TAR_EXECUTABLE ${TAR_EXECUTABLE} CACHE STRING "")

	if (TAR_EXECUTABLE)
		message(STATUS "Found tar")

		if (VERBOSE)
			message(STATUS "Path: ${TAR_EXECUTABLE}")
		endif (VERBOSE)
	else (TAR_EXECUTABLE)
		message(FATAL_ERROR "tar not found")
	endif (TAR_EXECUTABLE)

	message("")
endif (NOT TAR_EXECUTABLE)