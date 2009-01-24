# - This module determines the thread library of the system.
# The following variables are set
#  PTHREAD_FOUND - system has pthreads
#  PTHREAD_INCLUDE_DIR - the pthreads include directory
#  PTHREAD_LIBRARIES - the libraries needed to use pthreads

INCLUDE (CheckIncludeFile)
INCLUDE (CheckIncludeFiles)
INCLUDE (CheckLibraryExists)

# Do we have pthreads?
FIND_PATH(PTHREAD_INCLUDE_DIR pthread.h)
MARK_AS_ADVANCED(PTHREAD_INCLUDE_DIR)

IF(PTHREAD_INCLUDE_DIR)
	# We have pthread.h
	# Let's check for the library now.

	# Do we have -lpthreads
	FIND_LIBRARY(PTHREAD_LIBRARIES "pthreads")

	IF (NOT PTHREAD_LIBRARIES)
		FIND_LIBRARY(PTHREAD_LIBRARIES "pthread")
	ENDIF (NOT PTHREAD_LIBRARIES)

	IF (NOT PTHREAD_LIBRARIES AND WIN32)
		FIND_LIBRARY(PTHREAD_LIBRARIES "pthreadGC2")
	ENDIF (NOT PTHREAD_LIBRARIES AND WIN32)

	IF (NOT PTHREAD_LIBRARIES)
		MESSAGE(STATUS "Check if compiler accepts -pthread")

		TRY_RUN(THREADS_PTHREAD_ARG THREADS_HAVE_PTHREAD_ARG
			${CMAKE_BINARY_DIR}
			${CMAKE_ROOT}/Modules/CheckForPthreads.c
			CMAKE_FLAGS -DLINK_LIBRARIES:STRING=-pthread
			OUTPUT_VARIABLE OUTPUT)

		IF (THREADS_HAVE_PTHREAD_ARG)
			IF(THREADS_PTHREAD_ARG MATCHES "^2$")
				MESSAGE(STATUS "Check if compiler accepts -pthread - yes")
			ELSE (THREADS_PTHREAD_ARG MATCHES "^2$")
				MESSAGE(STATUS "Check if compiler accepts -pthread - no")

				FILE(APPEND 
					${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
					"Determining if compiler accepts -pthread returned ${THREADS_PTHREAD_ARG} instead of 2. The compiler had the following output:\n${OUTPUT}\n\n")
			ENDIF (THREADS_PTHREAD_ARG MATCHES "^2$")
		ELSE (THREADS_HAVE_PTHREAD_ARG)
			MESSAGE(STATUS "Check if compiler accepts -pthread - no")

			FILE(APPEND 
				${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
				"Determining if compiler accepts -pthread failed with the following output:\n${OUTPUT}\n\n")
		ENDIF (THREADS_HAVE_PTHREAD_ARG)

		IF (THREADS_HAVE_PTHREAD_ARG)
			SET(PTHREAD_LIBRARIES "-pthread")
		ENDIF (THREADS_HAVE_PTHREAD_ARG)
	ENDIF (NOT PTHREAD_LIBRARIES)

	MARK_AS_ADVANCED(PTHREAD_LIBRARIES)

	IF (PTHREAD_LIBRARIES)
		SET(PTHREAD_FOUND 1)
	ENDIF (PTHREAD_LIBRARIES)
ENDIF(PTHREAD_INCLUDE_DIR)