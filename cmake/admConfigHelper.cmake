MACRO(PRINT_LIBRARY_INFO libraryName libraryDetected compilerFlags linkerFlags)
	IF (${libraryDetected})
		MESSAGE(STATUS "Found ${libraryName}")

		IF (VERBOSE)
			SET(_compilerFlags "")
			APPEND_FLAGS(_compilerFlags ${compilerFlags})

			SET(_linkerFlags "")
			APPEND_FLAGS(_linkerFlags ${linkerFlags})

			MESSAGE(STATUS "Compiler Flags:${_compilerFlags}")
			MESSAGE(STATUS "Linker Flags  :${_linkerFlags}")
		ENDIF (VERBOSE)
	ELSE (${libraryDetected})
		MESSAGE(${ARGV4} "Could not find ${libraryName}")
	ENDIF (${libraryDetected})
ENDMACRO(PRINT_LIBRARY_INFO)


MACRO(SDLify _source)
	IF (SDL_FOUND)
		SET_SOURCE_FILES_PROPERTIES(${_source} PROPERTIES COMPILE_FLAGS "-I${SDL_INCLUDE_DIR}")
	ENDIF (SDL_FOUND)
ENDMACRO(SDLify)


# ARGV2 = library to check
# ARGV3 = function to check
# ARVG4 = extra required libs
MACRO(FIND_HEADER_AND_LIB prefix headerFile)
	IF (NOT DEFINED ${prefix}_FOUND)
		SET(${prefix}_FOUND 0 CACHE INTERNAL "")
		SET(_proceed 1)

		IF (NOT ${headerFile} STREQUAL "")
			FIND_PATH(${prefix}_INCLUDE_DIR ${headerFile})
			MARK_AS_ADVANCED(${prefix}_INCLUDE_DIR)

			IF (${prefix}_INCLUDE_DIR)
				MESSAGE(STATUS "Found ${headerFile}")
			ELSE (${prefix}_INCLUDE_DIR)
				SET(_proceed 0)
				MESSAGE("Could not find ${headerFile}")
			ENDIF (${prefix}_INCLUDE_DIR)
		ENDIF (NOT ${headerFile} STREQUAL "")

		IF (_proceed AND NOT ${ARGV2} STREQUAL "")
			FIND_LIBRARY(${prefix}_LIBRARY_DIR ${ARGV2})
			MARK_AS_ADVANCED(${prefix}_LIBRARY_DIR)

			IF (${prefix}_LIBRARY_DIR)
				MESSAGE(STATUS "Found ${ARGV2} library")

				IF (NOT ${ARGV3} STREQUAL "")
					ADM_CHECK_FUNCTION_EXISTS(${ARGV3} "${${prefix}_LIBRARY_DIR}" ${prefix}_FUNCTION_FOUND "${ARGV4}")

					IF (${prefix}_FUNCTION_FOUND)
						SET(${prefix}_FOUND 1 CACHE INTERNAL "")
					ENDIF (${prefix}_FUNCTION_FOUND)
				ELSE (NOT ${ARGV3} STREQUAL "")
					SET(${prefix}_FOUND 1 CACHE INTERNAL "")
				ENDIF (NOT ${ARGV3} STREQUAL "")
			ELSE (${prefix}_LIBRARY_DIR)
				MESSAGE(STATUS "Cound not find ${ARGV2} library")
			ENDIF (${prefix}_LIBRARY_DIR)
		ELSE (_proceed AND NOT ${ARGV2} STREQUAL "")
			SET(${prefix}_FOUND ${_proceed} CACHE INTERNAL "")
		ENDIF (_proceed AND NOT ${ARGV2} STREQUAL "")
	ENDIF (NOT DEFINED ${prefix}_FOUND)
ENDMACRO(FIND_HEADER_AND_LIB)


MACRO (ADM_COMPILE _file _def _include _lib _varToSet _output)
	IF (NOT DEFINED ${_varToSet}_COMPILED)
		SET(${_varToSet}_COMPILED 1 CACHE INTERNAL "")

		TRY_COMPILE(${_varToSet}
			  ${CMAKE_BINARY_DIR}
			  ${AVIDEMUX_TOP_SOURCE_DIR}/cmake_compile_check/${_file}
			  CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${_include}" "-DLINK_LIBRARIES:STRING=${_lib}"
			  COMPILE_DEFINITIONS ${_def}
			  OUTPUT_VARIABLE ${_output})
	ENDIF (NOT DEFINED ${_varToSet}_COMPILED)
ENDMACRO (ADM_COMPILE)


#ARGV3 = extra libraries
#ARGV4 = extra compile flags
MACRO (ADM_CHECK_FUNCTION_EXISTS _function _lib _varToSet)
	SET(CHECK_FUNCTION_DEFINE "-DCHECK_FUNCTION_EXISTS=${_function}" ${ARGV4})
	SET(CHECK_FUNCTION_LIB ${_lib} ${ARGV3})

	ADM_COMPILE(CheckFunctionExists.c "${CHECK_FUNCTION_DEFINE}" "" "${CHECK_FUNCTION_LIB}" ${_varToSet} OUTPUT)

	IF (${_varToSet})
		MESSAGE(STATUS "Found ${_function} in ${_lib}")
	ELSE (${_varToSet})
		MESSAGE(STATUS "Could not find ${_function} in ${_lib}")
		
		IF (VERBOSE)
			MESSAGE(${OUTPUT})
		ENDIF (VERBOSE)
	ENDIF (${_varToSet})	
ENDMACRO (ADM_CHECK_FUNCTION_EXISTS)


MACRO (CHECK_CFLAGS_REQUIRED _file _cflags _include _lib _varToSet)
	IF (NOT DEFINED ${_varToSet}_CFLAGS_CHECKED)
		SET(${_varToSet}_CFLAGS_CHECKED 1 CACHE INTERNAL "")

		ADM_COMPILE(${_file} ${_cflags} ${_include} ${_lib} ${_varToSet}_COMPILE_WITH logwith)
		ADM_COMPILE(${_file} "" ${_include} ${_lib} ${_varToSet}_COMPILE_WITHOUT logwithout)

		IF (${_varToSet}_COMPILE_WITH AND NOT ${_varToSet}_COMPILE_WITHOUT)
			SET(${_varToSet} 1 CACHE INTERNAL "")

			IF (VERBOSE)
				MESSAGE(STATUS "(${_cflags}) required")
			ENDIF (VERBOSE)
		ELSE (${_varToSet}_COMPILE_WITH AND NOT ${_varToSet}_COMPILE_WITHOUT)
			IF (NOT ${_varToSet}_COMPILE_WITH AND ${_varToSet}_COMPILE_WITHOUT)
				IF (VERBOSE)
					MESSAGE (STATUS "(${_cflags}) not required")
				ENDIF (VERBOSE)
			ELSE (NOT ${_varToSet}_COMPILE_WITH AND ${_varToSet}_COMPILE_WITHOUT)
				MESSAGE(STATUS "Inconsistent compiler output with: ${${_varToSet}_COMPILE_WITH}, without: ${${_varToSet}_COMPILE_WITHOUT}")
				MESSAGE(STATUS "${logwith}")
				MESSAGE(STATUS "")
				MESSAGE(STATUS "${logwithout}")
			endif(NOT ${_varToSet}_COMPILE_WITH AND ${_varToSet}_COMPILE_WITHOUT)
		ENDIF (${_varToSet}_COMPILE_WITH AND NOT ${_varToSet}_COMPILE_WITHOUT)
	ENDIF (NOT DEFINED ${_varToSet}_CFLAGS_CHECKED)
ENDMACRO (CHECK_CFLAGS_REQUIRED)


MACRO (APPEND_FLAGS _flags)
	IF (NOT ${_flags})
		SET(${_flags} "")
	ENDIF (NOT ${_flags})

	FOREACH (_flag ${ARGN})
		SET(${_flags} "${${_flags}} ${_flag}")
	ENDFOREACH (_flag ${ARGN})

	IF (${firstChar} AND ${firstChar} STREQUAL " ")
		STRING(LENGTH "${${_flags}}" stringLength)
		MATH(EXPR stringLength "${stringLength} - 1")
		STRING(SUBSTRING "${${_flags}}" 1 ${stringLength} ${_flags})
	ENDIF (${firstChar} AND ${firstChar} STREQUAL " ")
ENDMACRO (APPEND_FLAGS)


#ARGV1 = flags
MACRO (ADD_SOURCE_CFLAGS _target)
	GET_SOURCE_FILE_PROPERTY(_flags ${_target} COMPILE_FLAGS)
	APPEND_FLAGS(_flags ${ARGN})
	SET_SOURCE_FILES_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS "${_flags}")   
ENDMACRO (ADD_SOURCE_CFLAGS)


MACRO (ADD_TARGET_CFLAGS _target)
	GET_TARGET_PROPERTY(_flags ${_target} COMPILE_FLAGS)
	APPEND_FLAGS(_flags ${ARGN})
	SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS "${_flags}")
ENDMACRO (ADD_TARGET_CFLAGS)


MACRO (ADD_TARGET_DEFINITIONS _target)
	SET(newflag)
	GET_TARGET_PROPERTY(_flags ${_target} COMPILE_FLAGS)

	FOREACH (_def ${ARGN})
		IF (${_def})
			APPEND_FLAGS(_flags "-D${_def}")
			SET(newflag 1)
		ENDIF (${_def})
	ENDFOREACH (_def ${ARGN})

	IF (newflag)
		SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS "${_flags}")
	ENDIF (newflag)
ENDMACRO (ADD_TARGET_DEFINITIONS)


MACRO (ADD_CFLAGS_GTK_TARGET _target)
	IF (ADM_UI_GTK)
		ADD_TARGET_CFLAGS(${_target}_gtk ${ARGN})
	ENDIF (ADM_UI_GTK)
ENDMACRO (ADD_CFLAGS_GTK_TARGET)


MACRO (ADD_CFLAGS_QT4_TARGET _target)
	IF (ADM_UI_QT4)
		ADD_TARGET_CFLAGS(${_target}_qt4 ${ARGN})
	ENDIF (ADM_UI_QT4)
ENDMACRO (ADD_CFLAGS_QT4_TARGET)


MACRO (ADD_CFLAGS_ALL_TARGETS _target)
	ADD_TARGET_CFLAGS(${_target}_cli ${ARGN})
	ADD_CFLAGS_GTK_TARGET(${_target} ${ARGN})
	ADD_CFLAGS_QT4_TARGET(${_target} ${ARGN})
ENDMACRO (ADD_CFLAGS_ALL_TARGETS)


MACRO (ADD_ADM_LIB_CLI_TARGET _libName)
	ADD_LIBRARY(${_libName}_cli STATIC ${ARGN})
	ADD_TARGET_CFLAGS(${_libName}_cli "-I${CMAKE_BINARY_DIR}/config/cli")
	ADD_TARGET_CFLAGS(${_libName}_cli "-DADM_UI_TYPE_BUILD=1")
ENDMACRO (ADD_ADM_LIB_CLI_TARGET)


MACRO (ADD_ADM_LIB_GTK_TARGET _libName)
	IF (ADM_UI_GTK)
		ADD_LIBRARY(${_libName}_gtk STATIC ${ARGN})
		ADD_TARGET_CFLAGS(${_libName}_gtk "-I${CMAKE_BINARY_DIR}/config/gtk")
		ADD_TARGET_CFLAGS(${_libName}_gtk "-DADM_UI_TYPE_BUILD=2")
	ENDIF (ADM_UI_GTK)
ENDMACRO (ADD_ADM_LIB_GTK_TARGET)


MACRO (ADD_ADM_LIB_QT4_TARGET _libName)
	IF (ADM_UI_QT4)
		ADD_LIBRARY(${_libName}_qt4 STATIC ${ARGN})
		ADD_TARGET_CFLAGS(${_libName}_qt4 "-I${CMAKE_BINARY_DIR}/config/qt4 ")
		ADD_TARGET_CFLAGS(${_libName}_qt4 "-DADM_UI_TYPE_BUILD=4")
	ENDIF (ADM_UI_QT4)
ENDMACRO (ADD_ADM_LIB_QT4_TARGET)


MACRO (ADD_ADM_SHARED_LIB_CLI_TARGET _libName)
	ADD_LIBRARY(${_libName}_cli SHARED ${ARGN})
	ADD_TARGET_CFLAGS(${_libName}_cli "-I${CMAKE_BINARY_DIR}/config/cli")
	ADD_TARGET_CFLAGS(${_libName}_cli "-DADM_UI_TYPE_BUILD=1")
ENDMACRO (ADD_ADM_SHARED_LIB_CLI_TARGET)


MACRO (ADD_ADM_SHARED_LIB_GTK_TARGET _libName)
	IF (ADM_UI_GTK)
		ADD_LIBRARY(${_libName}_gtk SHARED ${ARGN})
		ADD_TARGET_CFLAGS(${_libName}_gtk "-I${CMAKE_BINARY_DIR}/config/gtk")
		ADD_TARGET_CFLAGS(${_libName}_gtk "-DADM_UI_TYPE_BUILD=2")
	ENDIF (ADM_UI_GTK)
ENDMACRO (ADD_ADM_SHARED_LIB_GTK_TARGET)


MACRO (ADD_ADM_SHARED_LIB_QT4_TARGET _libName)
	IF (ADM_UI_QT4)
		ADD_LIBRARY(${_libName}_qt4 SHARED ${ARGN})
		ADD_TARGET_CFLAGS(${_libName}_qt4 "-I${CMAKE_BINARY_DIR}/config/qt4")
		ADD_TARGET_CFLAGS(${_libName}_qt4 "-DADM_UI_TYPE_BUILD=4")
	ENDIF (ADM_UI_QT4)
ENDMACRO (ADD_ADM_SHARED_LIB_QT4_TARGET)



MACRO (ADD_ADM_LIB_ALL_TARGETS _libName)
	ADD_ADM_LIB_CLI_TARGET(${_libName} ${ARGN})
	ADD_ADM_LIB_GTK_TARGET(${_libName} ${ARGN})
	ADD_ADM_LIB_QT4_TARGET(${_libName} ${ARGN})
ENDMACRO (ADD_ADM_LIB_ALL_TARGETS)


MACRO (ADD_ADM_SHARED_LIB_ALL_TARGETS _libName)
	ADD_ADM_SHARED_LIB_CLI_TARGET(${_libName} ${ARGN})
	ADD_ADM_SHARED_LIB_GTK_TARGET(${_libName} ${ARGN})
	ADD_ADM_SHARED_LIB_QT4_TARGET(${_libName} ${ARGN})
ENDMACRO (ADD_ADM_SHARED_LIB_ALL_TARGETS)

MACRO (ADM_TARGET_LINK_LIBARIES_ALL_TARGET _libName)
	IF (ADM_UI_QT4)
		TARGET_LINK_LIBRARIES( ${_libName}_qt4 ${ARGN})
	ENDIF (ADM_UI_QT4)
	IF (ADM_UI_GTK)
		TARGET_LINK_LIBRARIES( ${_libName}_gtk ${ARGN})
	ENDIF (ADM_UI_GTK)
	TARGET_LINK_LIBRARIES( ${_libName}_cli ${ARGN})
ENDMACRO (ADM_TARGET_LINK_LIBARIES_ALL_TARGET)

MACRO (ADD_TARGET_LDFLAGS _target)
	SET(_flags)
	APPEND_FLAGS(_flags ${ARGN})

	TARGET_LINK_LIBRARIES(${_target} ${_flags})
ENDMACRO (ADD_TARGET_LDFLAGS)
