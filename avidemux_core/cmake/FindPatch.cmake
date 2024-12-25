IF(NOT PATCH_EXECUTABLE)
	MESSAGE(STATUS "Checking for patch")
	MESSAGE(STATUS "******************")

	find_program(PATCH_EXECUTABLE patch)
	SET(PATCH_EXECUTABLE ${PATCH_EXECUTABLE} CACHE STRING "")

	IF(PATCH_EXECUTABLE)
		MESSAGE(STATUS "Found patch")
		
		IF(VERBOSE)
			MESSAGE(STATUS "Path: ${PATCH_EXECUTABLE}")
		ENDIF()
	ELSE()
		MESSAGE(FATAL_ERROR "patch not found")
	ENDIF()

	MESSAGE("")
ENDIF()

macro(patch_file baseDir patchFile)
	execute_process(COMMAND ${PATCH_EXECUTABLE} -p0 -i "${patchFile}"
					WORKING_DIRECTORY "${baseDir}"
                                        RESULT_VARIABLE   res
        )
        if(res)
                MESSAGE(FATAL_ERROR "Patch failed")
        ENDIF()
ENDMACRO()

macro(patch_file_p1 baseDir patchFile)
	execute_process(COMMAND ${PATCH_EXECUTABLE} -p1  -i "${patchFile}"
					WORKING_DIRECTORY "${baseDir}"
                                        RESULT_VARIABLE   res
				)
        if(res)
                MESSAGE(FATAL_ERROR "Patch failed")
        ENDIF()
ENDMACRO()
