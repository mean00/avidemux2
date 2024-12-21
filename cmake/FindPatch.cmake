if (NOT PATCH_EXECUTABLE)
	message(STATUS "Checking for patch")
	message(STATUS "******************")

	find_program(PATCH_EXECUTABLE patch)
	set(PATCH_EXECUTABLE ${PATCH_EXECUTABLE} CACHE STRING "")

	if (PATCH_EXECUTABLE)
		message(STATUS "Found patch")
		
		if (VERBOSE)
			message(STATUS "Path: ${PATCH_EXECUTABLE}")
		endif (VERBOSE)
	else (PATCH_EXECUTABLE)
		message(FATAL_ERROR "patch not found")
	endif (PATCH_EXECUTABLE)

	message("")
endif (NOT PATCH_EXECUTABLE)

macro(patch_file baseDir patchFile)
	execute_process(COMMAND ${PATCH_EXECUTABLE} -p0 -i "${patchFile}"
					WORKING_DIRECTORY "${baseDir}"
                                        RESULT_VARIABLE   res
        )
        if(res)
                MESSAGE(FATAL_ERROR "Patch failed")
        endif(res)
endmacro(patch_file)

macro(patch_file_p1 baseDir patchFile)
	execute_process(COMMAND ${PATCH_EXECUTABLE} -p1  -i "${patchFile}"
					WORKING_DIRECTORY "${baseDir}"
                                        RESULT_VARIABLE   res
				)
        if(res)
                MESSAGE(FATAL_ERROR "Patch failed")
        endif(res)
endmacro(patch_file_p1)
