if (NOT BASH_EXECUTABLE)
	message(STATUS "Checking for Bourne shell")
	message(STATUS "*************************")

	find_program(BASH_EXECUTABLE
		bash
		${BASH_DIR}
		/bin
		/usr/bin 
		/usr/local/bin
		/sbin)
	set(BASH_EXECUTABLE ${BASH_EXECUTABLE} CACHE STRING "")

	if (BASH_EXECUTABLE)
		message(STATUS "Found Bourne shell")
		
		if (VERBOSE)
			message(STATUS "Path: ${BASH_EXECUTABLE}")
		endif (VERBOSE)
	else (BASH_EXECUTABLE)
		message(FATAL_ERROR "Bourne shell not found")
	endif (BASH_EXECUTABLE)

	message("")
endif (NOT BASH_EXECUTABLE)