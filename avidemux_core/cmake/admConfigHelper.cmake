MACRO(PRINT_LIBRARY_INFO libraryName libraryDetected compilerFlags linkerFlags)
  IF(${libraryDetected})
    MESSAGE(STATUS "Found ${libraryName}")

    IF(VERBOSE)
      SET(_compilerFlags "")
      APPEND_FLAGS(_compilerFlags ${compilerFlags})

      SET(_linkerFlags "")
      APPEND_FLAGS(_linkerFlags ${linkerFlags})

      MESSAGE(STATUS "Compiler Flags:${_compilerFlags}")
      MESSAGE(STATUS "Linker Flags  :${_linkerFlags}")
    ENDIF()
  ELSE()
    MESSAGE(STATUS ${ARGV4} "Could not find ${libraryName}")
  ENDIF()
ENDMACRO()


MACRO(SDLify _source)
  IF(SDL2_FOUND)
    SET_SOURCE_FILES_PROPERTIES(${_source} PROPERTIES COMPILE_FLAGS "-I${SDL2_INCLUDE_DIR} ${SDL2_CFLAGS}")
  ENDIF()
ENDMACRO()


# ARGV2 = library to check
# ARGV3 = function to check
# ARVG4 = extra required libs
MACRO(FIND_HEADER_AND_LIB prefix headerFile)
  IF(NOT DEFINED ${prefix}_FOUND)
    SET(${prefix}_FOUND 0 CACHE INTERNAL "")
    SET(_proceed 1)

    IF(NOT ${headerFile} STREQUAL "")
      FIND_PATH(${prefix}_INCLUDE_DIR ${headerFile}
          PATHS /usr/include/x86_64-linux-gnu) # Needed for 64 bits linux
      MARK_AS_ADVANCED(${prefix}_INCLUDE_DIR)

      IF(${prefix}_INCLUDE_DIR)
        MESSAGE(STATUS "Found ${headerFile}")
      ELSE()
        SET(_proceed 0)
        MESSAGE(STATUS "Could not find ${headerFile}")
      ENDIF()
    ENDIF()

    IF(_proceed AND NOT ${ARGV2} STREQUAL "")
      # On Mac, search firts in port libs, else we might use an older lib coming from the system (e.g. iconv)
      IF(APPLE)
        FIND_LIBRARY(${prefix}_LIBRARY_DIR ${ARGV2} PATHS /opt/local/lib NO_DEFAULT_PATH) # needed for port stuff
      ENDIF()
      FIND_LIBRARY(${prefix}_LIBRARY_DIR ${ARGV2})
      MARK_AS_ADVANCED(${prefix}_LIBRARY_DIR)

      IF(${prefix}_LIBRARY_DIR)
        MESSAGE(STATUS "Found ${ARGV2} library")

        IF(NOT ${ARGV3} STREQUAL "")
          ADM_CHECK_FUNCTION_EXISTS(${ARGV3} "${${prefix}_LIBRARY_DIR}" ${prefix}_FUNCTION_FOUND "${ARGV4}")

          IF(${prefix}_FUNCTION_FOUND)
            SET(${prefix}_FOUND 1 CACHE INTERNAL "")
          ENDIF()
        ELSE()
          SET(${prefix}_FOUND 1 CACHE INTERNAL "")
        ENDIF()
      ELSE()
        MESSAGE(STATUS "Cound not find ${ARGV2} library")
      ENDIF()
    ELSE()
      SET(${prefix}_FOUND ${_proceed} CACHE INTERNAL "")
    ENDIF()
  ENDIF()
ENDMACRO()


MACRO(ADM_COMPILE _file _def _include _lib _varToSet _output)
  #MESSAGE(STATUS " ADM_compile <${_file}>")
  IF(AVIDEMUX_THIS_IS_CORE)
    SET(src ${AVIDEMUX_CORE_SOURCE_DIR}/cmake/cmake_compile_check/${_file})
  ELSE()
    SET(src ${ADM_CMAKE_DIR}/cmake_compile_check/${_file})
  ENDIF()
  #MESSAGE(STATUS " Compiling <${src}>")
  IF(NOT DEFINED ${_varToSet}_COMPILED)
    SET(${_varToSet}_COMPILED 1 CACHE INTERNAL "")
    TRY_COMPILE(${_varToSet}
      ${CMAKE_BINARY_DIR}
      ${src}
      CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${_include}" "-DLINK_LIBRARIES:STRING=${_lib}"
      COMPILE_DEFINITIONS ${_def}
      OUTPUT_VARIABLE ${_output})
    #MESSAGE(STATUS "output=${output}")
  ENDIF()
ENDMACRO()


#
# pkg-config hands out GNU style link flags ("-L<dir>;-l<name>"). link.exe does not
# understand them: it turns them into '/LF:...' and '/lvpx', ignores them (LNK4044) and
# the link then fails with an unresolved symbol - so function probes silently conclude
# "not found" and the matching plugin is disabled. On MSVC only, rewrite those flags into
# absolute .lib paths, which is what LINK_LIBRARIES expects. Tokens with no MSVC
# equivalent (e.g. -lm) are dropped.
#
# Private helper: use ADM_PKG_CHECK_MODULES() below, do not call this directly.
# No-op for every non-MSVC toolchain: Linux / macOS / MinGW keep the GNU flags untouched.
#
MACRO(_ADM_PKG_LDFLAGS_TO_MSVC _varName)
  IF(MSVC AND "${${_varName}}" MATCHES "(^|[ ;])-[Ll]")
    STRING(REPLACE ";" " " _adm_flags "${${_varName}}")
    SEPARATE_ARGUMENTS(_adm_flags UNIX_COMMAND "${_adm_flags}")

    SET(_adm_dirs "")
    SET(_adm_libs "")
    SET(_adm_other "")
    FOREACH(_adm_token ${_adm_flags})
      IF(_adm_token MATCHES "^-L(.+)$")
        LIST(APPEND _adm_dirs "${CMAKE_MATCH_1}")
      ELSEIF(_adm_token MATCHES "^-l(.+)$")
        LIST(APPEND _adm_libs "${CMAKE_MATCH_1}")
      ELSE()
        LIST(APPEND _adm_other "${_adm_token}")
      ENDIF()
    ENDFOREACH()

    IF(_adm_dirs)
      LIST(GET _adm_dirs 0 _adm_first_dir)
    ELSE()
      SET(_adm_first_dir "")
    ENDIF()

    SET(_adm_result ${_adm_other})
    FOREACH(_adm_lib ${_adm_libs})
      UNSET(_adm_resolved)
      UNSET(_adm_resolved CACHE)
      IF(_adm_dirs)
        FIND_LIBRARY(_adm_resolved NAMES ${_adm_lib} PATHS ${_adm_dirs} NO_DEFAULT_PATH)
      ENDIF()
      IF(NOT _adm_resolved)
        # not where pkg-config said: fall back to the toolchain search path
        FIND_LIBRARY(_adm_resolved NAMES ${_adm_lib})
      ENDIF()
      IF(NOT _adm_resolved AND _adm_first_dir AND EXISTS "${_adm_first_dir}/${_adm_lib}.lib")
        SET(_adm_resolved "${_adm_first_dir}/${_adm_lib}.lib")
      ENDIF()
      UNSET(_adm_resolved CACHE)
      IF(_adm_resolved)
        LIST(APPEND _adm_result "${_adm_resolved}")
      ELSE()
        MESSAGE(STATUS "    [msvc] no import library for '-l${_adm_lib}', dropping it")
      ENDIF()
    ENDFOREACH()

    SET(${_varName} "${_adm_result}")
    MESSAGE(STATUS "    [msvc] ${_varName} -> ${${_varName}}")
  ENDIF()
ENDMACRO()

#
# Platform neutral drop-in replacement for PKG_CHECK_MODULES().
#
# Same syntax, same semantics, same outputs in the caller's scope
# (<prefix>_FOUND/_VERSION/_INCLUDE_DIRS/_LIBRARY_DIRS/_LIBRARIES/_LINK_LIBRARIES/
# _CFLAGS/_LDFLAGS/_LDFLAGS_OTHER), so `REQUIRED`, `QUIET` and version constraints
# (e.g. aom>=3.2.0) all keep working. The only difference: on MSVC the link flags are
# converted to something link.exe understands (see above).
#
# Use this instead of PKG_CHECK_MODULES() everywhere, so that no platform specific code
# has to leak into the individual checks.
#
MACRO(ADM_PKG_CHECK_MODULES _prefix)
  PKG_CHECK_MODULES(${_prefix} ${ARGN})
  IF(MSVC)
    _ADM_PKG_LDFLAGS_TO_MSVC(${_prefix}_LDFLAGS)
  ENDIF()
ENDMACRO()


#ARGV3 = extra libraries
#ARGV4 = extra compile flags
MACRO(ADM_CHECK_FUNCTION_EXISTS _function _lib _varToSet)
  SET(CHECK_FUNCTION_DEFINE "-DCHECK_FUNCTION_EXISTS=${_function}" ${ARGV4})
  SET(CHECK_FUNCTION_LIB ${_lib} ${ARGV3})

  ADM_COMPILE(CheckFunctionExists.c "${CHECK_FUNCTION_DEFINE}" "" "${CHECK_FUNCTION_LIB}" ${_varToSet} OUTPUT)

  IF(${_varToSet})
    MESSAGE(STATUS "Found ${_function} in ${_lib}")
  ELSE()
    MESSAGE(STATUS "Could not find ${_function} in ${_lib}")

    IF(VERBOSE)
      MESSAGE(STATUS ${OUTPUT})
    ENDIF()
  ENDIF()
ENDMACRO()


MACRO(CHECK_CFLAGS_REQUIRED _file _cflags _include _lib _varToSet)
  IF(NOT DEFINED ${_varToSet}_CFLAGS_CHECKED)
    SET(${_varToSet}_CFLAGS_CHECKED 1 CACHE INTERNAL "")

    ADM_COMPILE(${_file} ${_cflags} ${_include} ${_lib} ${_varToSet}_COMPILE_WITH logwith)
    ADM_COMPILE(${_file} "" ${_include} ${_lib} ${_varToSet}_COMPILE_WITHOUT logwithout)

    IF(${_varToSet}_COMPILE_WITH AND NOT ${_varToSet}_COMPILE_WITHOUT)
      SET(${_varToSet} 1 CACHE INTERNAL "")

      IF(VERBOSE)
        MESSAGE(STATUS "(${_cflags}) required")
      ENDIF()
    ELSE()
      IF(NOT ${_varToSet}_COMPILE_WITH AND ${_varToSet}_COMPILE_WITHOUT)
        IF(VERBOSE)
          MESSAGE (STATUS "(${_cflags}) not required")
        ENDIF()
      ELSE()
        MESSAGE(STATUS "Inconsistent compiler output with: ${${_varToSet}_COMPILE_WITH}, without: ${${_varToSet}_COMPILE_WITHOUT}")
        MESSAGE(STATUS "${logwith}")
        MESSAGE(STATUS "")
        MESSAGE(STATUS "${logwithout}")
      ENDIF()
    ENDIF()
  ENDIF()
ENDMACRO()


MACRO(APPEND_FLAGS _flags)
  IF(NOT ${_flags})
    SET(${_flags} "")
  ENDIF()

  FOREACH(_flag ${ARGN})
    SET(${_flags} "${${_flags}} ${_flag}")
  ENDFOREACH(_flag ${ARGN})

  IF(${firstChar} AND ${firstChar} STREQUAL " ")
    STRING(LENGTH "${${_flags}}" stringLength)
    MATH(EXPR stringLength "${stringLength} - 1")
    STRING(SUBSTRING "${${_flags}}" 1 ${stringLength} ${_flags})
  ENDIF()
ENDMACRO()


#ARGV1 = flags
MACRO(ADD_SOURCE_CFLAGS _target)
  GET_SOURCE_FILE_PROPERTY(_flags ${_target} COMPILE_FLAGS)
  APPEND_FLAGS(_flags ${ARGN})
  SET_SOURCE_FILES_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS "${_flags}")
ENDMACRO()


MACRO(ADD_TARGET_CFLAGS _target)
  GET_TARGET_PROPERTY(_flags ${_target} COMPILE_FLAGS)
  APPEND_FLAGS(_flags ${ARGN})
  SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS "${_flags}")
ENDMACRO()


MACRO(ADD_TARGET_DEFINITIONS _target)
  SET(newflag)
  GET_TARGET_PROPERTY(_flags ${_target} COMPILE_FLAGS)

  FOREACH(_def ${ARGN})
    IF(${_def})
      APPEND_FLAGS(_flags "-D${_def}")
      SET(newflag 1)
    ENDIF()
  ENDFOREACH(_def ${ARGN})

  IF(newflag)
    SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS "${_flags}")
  ENDIF()
ENDMACRO()

MACRO(DUMP_ALL_VARS)
  get_cmake_property(_variableNames VARIABLES)
  LIST(SORT _variableNames)
  FOREACH(_variableName ${_variableNames})
    MESSAGE(STATUS "${_variableName}=${${_variableName}}")
  ENDFOREACH()
ENDMACRO()

