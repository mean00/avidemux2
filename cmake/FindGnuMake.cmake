if (NOT GNUMAKE_EXECUTABLE)
	message(STATUS "Checking for GNU Make")
	message(STATUS "*********************")

	find_program(GNUMAKE_EXECUTABLE
		make
		${GNUMAKE_DIR}
		/bin
		/usr/bin 
		/usr/local/bin
		/sbin)
	set(GNUMAKE_EXECUTABLE ${GNUMAKE_EXECUTABLE} CACHE STRING "")

	if (GNUMAKE_EXECUTABLE)
		message(STATUS "Found GNU Make")
		
		if (VERBOSE)
			message(STATUS "Path: ${GNUMAKE_EXECUTABLE}")
		endif (VERBOSE)
	else (GNUMAKE_EXECUTABLE)
		message(FATAL_ERROR "GNU Make not found")
	endif (GNUMAKE_EXECUTABLE)

	message("")
endif (NOT GNUMAKE_EXECUTABLE)