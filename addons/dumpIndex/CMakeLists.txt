SET(ADM_PROJECT tsDump)
#
SET(CMAKE_INSTALL_PREFIX /usr)
SET(AVIDEMUX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../..)
#

MESSAGE(STATUS "Checking for AVIDEMUX_SOURCE_DIR, AVIDEMUX_LIB_DIR, AVIDEMUX_INCLUDE_DIR...")
IF (NOT AVIDEMUX_SOURCE_DIR)
	MESSAGE(FATAL_ERROR "Please add -DAVIDEMUX_SOURCE_DIR=path_to_avidemux_source. You just need the source tree.")
ELSE (NOT AVIDEMUX_TOP_SOURCE_DIR)
        # We need TOP_SOURCE_DIR 
	SET(AVIDEMUX_TOP_SOURCE_DIR "${AVIDEMUX_SOURCE_DIR}" CACHE STRING "")
ENDIF (NOT AVIDEMUX_SOURCE_DIR)


# Common definitions...
SET(CMAKE_MODULE_PATH "${AVIDEMUX_TOP_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
include(${AVIDEMUX_TOP_SOURCE_DIR}/cmake/commonCmakeApplication.cmake)
include(${AVIDEMUX_TOP_SOURCE_DIR}/cmake/admAsNeeded.cmake)
########################################
# Add include dirs
########################################
SET(AVIDEMUX_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
MARK_AS_ADVANCED(AVIDEMUX_INSTALL_DIR)
include(admInstallDir)
#########################################
# Get regular stuff from main app
#########################################
include(admCoreIncludes)
LINK_DIRECTORIES("${AVIDEMUX_LIB_DIR}")
INCLUDE_DIRECTORIES(${AVIDEMUX_INCLUDE_DIR}/avidemux/2.6/)

IF (FRESH_BUILD)
	MESSAGE("")
ENDIF (FRESH_BUILD)


SET(SRCS 
        main.cpp
        ADM_indexFile.cpp
        ADM_tsReadIndex.cpp
        )

ADD_EXECUTABLE(dumpIndex ${SRCS})
TARGET_LINK_LIBRARIES(dumpIndex ADM_core6)
