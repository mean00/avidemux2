macro(INITIALISE_SUMMARY_LISTS)
	foreach(componentType ${SUMMARY_LIST})
		GET_SUMMARY_LIST_NAME(listName ${componentType})
		unset(${listName} CACHE)
	endforeach(componentType)

	set(SUMMARY_LIST "" CACHE INTERNAL "")
endmacro(INITIALISE_SUMMARY_LISTS)

macro(GET_SUMMARY_LIST_NAME outVariable componentType)
	string(REPLACE " " "_" safeComponentType "${componentType}")
	set(${outVariable} "SUMMARY_${safeComponentType}")
endmacro(GET_SUMMARY_LIST_NAME)

macro(APPEND_SUMMARY_LIST componentType componentName componentFound)
	GET_SUMMARY_LIST_NAME(listName ${componentType})

	if (${listName})
		set(${listName} ${${listName}} "${componentName}=${componentFound}" CACHE INTERNAL "")
	else (${listName})
		if (SUMMARY_LIST)
			set(SUMMARY_LIST ${componentType} ${SUMMARY_LIST} CACHE INTERNAL "")
		else (SUMMARY_LIST)
			set(SUMMARY_LIST ${componentType} CACHE INTERNAL "")
		endif (SUMMARY_LIST)

		set(${listName} "${componentName}=${componentFound}" CACHE INTERNAL "")
	endif (${listName})
endmacro(APPEND_SUMMARY_LIST)

function(padString outVariable length padCharacter value)
    string(LENGTH "${value}" valueLength)
    math(EXPR padLength "${length} - ${valueLength} - 1")
    set(paddedValue ${value})

    if (padLength GREATER -1)
        foreach(dummy RANGE ${padLength})
            set(paddedValue "${paddedValue}${padCharacter}")
        endforeach(dummy)
    endif (padLength GREATER -1)

    set(${outVariable} "${paddedValue}" PARENT_SCOPE)
endfunction(padString)

macro(DISPLAY_SUMMARY_LIST)
	message("")
	message("**************************")
	message("***  Optional Library  ***")
	message("***      Summary       ***")
	message("**************************")

	foreach(componentType ${SUMMARY_LIST})
		GET_SUMMARY_LIST_NAME(listName ${componentType})

		padString(componentType 18 " " "${componentType}")
		message("*** ${componentType} ***")

		foreach(component ${${listName}})
			string(FIND "${component}" = valuePosition)

			string(SUBSTRING "${component}" 0 ${valuePosition} componentName)
			math(EXPR valuePosition "${valuePosition} + 1")
			string(SUBSTRING "${component}" ${valuePosition} -1 componentFound)

			if (${componentFound})
				set(componentSummary "Yes")
			else (${componentFound})
				set(componentSummary "No")
			endif (${componentFound})

			padString(componentName 14 " " "${componentName}")
			message("    ${componentName} ${componentSummary}")
		endforeach(component)
	endforeach(componentType)

	message("**************************")

	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		message("***    Debug Build     ***")
	else (CMAKE_BUILD_TYPE STREQUAL "Debug")
		message("***   Release Build    ***")
	endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

	message("**************************")
	message("")
endmacro(DISPLAY_SUMMARY_LIST)