###################################################
MESSAGE("#####################################")
MESSAGE("Custom videoFilter plugin template Started")
MESSAGE("#####################################")
MESSAGE("")
ADD_DEFINITIONS("-g")
SET(ADM_PROJECT AdmLogoVideoFilter)
#
SET(PLUGINS TRUE)
SET(DO_COMMON TRUE)
SET(AVIDEMUX_VERSION 2.6)                 # important !
SET(AVIDEMUX_EXTERNAL_BUILD True)         # important !
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0 FATAL_ERROR)
######################################################
# Try to locate & setup avidemux development files
######################################################

MESSAGE(STATUS "Checking for avidemux development files ..")

SET(ADM_HEADER_DIR ${CMAKE_INSTALL_PREFIX}/include/avidemux/${AVIDEMUX_VERSION})
SET(ADM_CMAKE_DIR  ${ADM_HEADER_DIR}/cmake)

# Common definitions...
SET(CMAKE_MODULE_PATH "${ADM_CMAKE_DIR}" "${CMAKE_MODULE_PATH}")
MESSAGE(STATUS  "Checking for avidemux include folder (i.e. CMAKE_INSTALL_PREFIX/include/avidemux/${AVIDEMUX_VERSION}")
if(NOT EXISTS "${ADM_HEADER_DIR}")
        MESSAGE(STATUS  "Make sure you installed all the files.\n i cannot find avidemux include folder.cmake .\nSet CMAKE_INSTALL_PREFIX to the install folder, current value is ${CMAKE_INSTALL_PREFIX}")
        MESSAGE(FATAL_ERROR  "Aborting")
endif(NOT EXISTS "${ADM_HEADER_DIR}")

MESSAGE(STATUS "Found avidemux include folder. good.")
MESSAGE(STATUS "Checking for cmake subfolder")

if(NOT EXISTS "${ADM_CMAKE_DIR}/admCoreIncludesExternalBuild.cmake")
        MESSAGE(STATUS  "Make sure you installed all the files.\n I cannot find content of the cmake subfolder .\n")
        MESSAGE(STATUS  "Set CMAKE_INSTALL_PREFIX to the install folder, current value is ${CMAKE_INSTALL_PREFIX}")
        MESSAGE(STATUS  "I was looking for admCoreIncludesExternalBuild.cmake in  ${ADM_CMAKE_DIR}")
        MESSAGE(FATAL_ERROR  "Aborting")
endif(NOT EXISTS "${ADM_CMAKE_DIR}/admCoreIncludesExternalBuild.cmake")
MESSAGE(STATUS "Found cmake subfolder.good.")
#
#
# Check QTX openGL
include(admConfigHelper)
include(admCheckQt5)
checkQt5()
#
include_directories( ${QT_QTOPENGL_INCLUDE_DIR} )

#
# Check with have the Qt4/5 header
#
MESSAGE(STATUS "Checking for avidemux openGl subfolder (${QT_EXTENSION})")
if(NOT EXISTS "${ADM_HEADER_DIR}/${QT_EXTENSION}/ADM_openGL/ADM_openGl.h")
        MESSAGE(STATUS  "Make sure you installed all the files.\n I cannot find avidemux QT folder .\n")
        MESSAGE(STATUS  "Set CMAKE_INSTALL_PREFIX to the install folder, current value is ${CMAKE_INSTALL_PREFIX}")
        MESSAGE(STATUS  "I was looking for ADM_openGL/ADM_openGl.h in  ${ADM_HEADER_DIR}/${QT_EXTENSION}/ADM_openGL/")
        MESSAGE(FATAL_ERROR  "Aborting")
endif(NOT EXISTS "${ADM_HEADER_DIR}/${QT_EXTENSION}/ADM_openGL/ADM_openGl.h")
include(${ADM_CMAKE_DIR}/admCoreIncludesExternalBuild.cmake)
include(${ADM_CMAKE_DIR}/admConfigHelper.cmake)
########################################
# Add include dirs
########################################
SET(AVIDEMUX_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
MARK_AS_ADVANCED(AVIDEMUX_INSTALL_DIR)
include(admInstallDir)
#########################################
# Get regular stuff from main app
#########################################
LINK_DIRECTORIES("${AVIDEMUX_SEARCH_LIB_DIR}")
INCLUDE_DIRECTORIES(${AVIDEMUX_SEARCH_INCLUDE_DIR}/avidemux/${AVIDEMUX_VERSION}/)
INCLUDE_DIRECTORIES(${ADM_HEADER_DIR}/${QT_EXTENSION})
INCLUDE_DIRECTORIES(${ADM_HEADER_DIR}/${QT_EXTENSION}/ADM_openGL)
INCLUDE_DIRECTORIES(${ADM_HEADER_DIR}/${QT_EXTENSION}/ADM_UIs)

IF (FRESH_BUILD)
	MESSAGE("")
ENDIF (FRESH_BUILD)

########################################
# Global options
########################################
OPTION(VERBOSE "" OFF)
################################################
# Now really define our plugin...
###############################################
INCLUDE(vf_plugin)
INCLUDE(vf_plugin_qt4gl)
ADD_DEFINITIONS( "-DADM_UI_TYPE_BUILD=4")

SET(ADM_vf_shader_SRCS shader.cpp)
SET(OOP GL)
ADD_VIDEO_FILTER(ADM_vf_shader_sample ${ADM_vf_shader_SRCS})
TARGET_LINK_LIBRARIES(ADM_vf_shader_sample  ADM_UI${QT_LIBRARY_EXTENSION}6   ADM_coreImageLoader6 ADM_openGL${QT_LIBRARY_EXTENSION}6 ${QT_QTOPENGL_LIBRARIES} )
TARGET_LINK_LIBRARIES(ADM_vf_shader_sample  ${OOP} )
INIT_VIDEO_FILTER(ADM_vf_shader_sample)
INSTALL_VIDEO_FILTER(ADM_vf_shader_sample ${QT_EXTENSION})
