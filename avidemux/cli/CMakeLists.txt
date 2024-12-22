CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

PROJECT(Avidemux_CLI)

SET(UI_TYPE ADM_UI_CLI)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake" "${CMAKE_MODULE_PATH}")
INCLUDE(avidemuxVersion)

MESSAGE("")
MESSAGE("#########################################")
MESSAGE("Configure for avidemux Cli Started")
MESSAGE("#########################################")
MESSAGE("")

MESSAGE(STATUS "Checking for avidemux development files ..")

IF(NOT FAKEROOT)
  SET(AVIDEMUX_FAKEROOT "")
ELSE()
  SET(AVIDEMUX_FAKEROOT "${FAKEROOT}")
ENDIF()

SET(ADM_HEADER_DIR ${AVIDEMUX_FAKEROOT}${CMAKE_INSTALL_PREFIX}/include/avidemux/${AVIDEMUX_API_VERSION})
SET(ADM_CMAKE_DIR  ${ADM_HEADER_DIR}/cmake)
# Common definitions...
SET(CMAKE_MODULE_PATH "${ADM_CMAKE_DIR}" "${CMAKE_MODULE_PATH}")
MESSAGE(STATUS  "Checking for avidemux include folder (i.e. CMAKE_INSTALL_PREFIX/include/avidemux/${AVIDEMUX_API_VERSION}")
IF(NOT EXISTS "${ADM_HEADER_DIR}")
  MESSAGE(STATUS  "Make sure you installed all the files.\n i cannot find avidemux include folder.cmake .\nSet CMAKE_INSTALL_PREFIX to the install folder, current value is ${CMAKE_INSTALL_PREFIX}")
  MESSAGE(FATAL_ERROR  "Aborting")
ENDIF()

MESSAGE(STATUS "Found avidemux include folder. good.")
MESSAGE(STATUS "Checking for cmake subfolder")

IF(NOT EXISTS "${ADM_CMAKE_DIR}/commonCmakeApplication.cmake")
  MESSAGE(STATUS  "Make sure you installed all the files.\n I cannot find content of the cmake subfolder .\n")
  MESSAGE(STATUS  "Set CMAKE_INSTALL_PREFIX to the install folder, current value is ${CMAKE_INSTALL_PREFIX}")
  MESSAGE(STATUS  "I was looking for commonCmakeApplication.cmake in  ${ADM_CMAKE_DIR}")
  MESSAGE(FATAL_ERROR  "Aborting")
ENDIF()
MESSAGE(STATUS "Found cmake subfolder.good.")


INCLUDE(commonCmakeApplication)
INCLUDE(admWindRes)
#
#
#
#
ADD_EXECUTABLE(avidemux3_cli)
TARGET_SOURCES(avidemux3_cli PRIVATE ${ADM_EXE_SRCS})
TARGET_LINK_LIBRARIES(avidemux3_cli PRIVATE adm_gettext)
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Config
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_CLI)
SET(CONFIG_HEADER_TYPE ADM_BUILD_CLI)
SET(UI_SUFFIX cli)

CONFIGURE_FILE("${ADM_CMAKE_DIR}/config.h.cmake" "${CMAKE_BINARY_DIR}/config/cli/config.h")
MESSAGE(STATUS "CLI config.h generated")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_BINARY_DIR}/config/cli/")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Add subdirectories
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
ADD_SUBDIRECTORY(../common ./common)
ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsCli)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfacesCli)

SDLify(../common/main.cpp)
IF(ADM_SUBVERSION)
  ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_SUBVERSION=\"${ADM_SUBVERSION}\"")
ENDIF()
ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_VERSION=\"${AVIDEMUX_VERSION}\"")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Version Info
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
IF(WIN32)
  WINDRESIFY(mainCLI  "${CMAKE_CURRENT_SOURCE_DIR}/../common/xpm/adm.ico" src "avidemux_cli" "Avidemux Video Editor, CLI Version")
  TARGET_SOURCES(avidemux3_cli PRIVATE ${src})
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Executable
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Construct common libraries
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

MACRO(LINKBUNCH lst)
  TARGET_LINK_LIBRARIES(avidemux3_cli PRIVATE ${lst} ${ARGN})
ENDMACRO()

TARGET_LINK_LIBRARIES(avidemux3_cli PRIVATE ADM_editor6_interface ADM_videoEncoder6_interface ADM_videoFilter6_interface ADM_commonUI6_interface)
LINKBUNCH(${commonLibs1})
TARGET_LINK_LIBRARIES(avidemux3_cli PRIVATE ADM_filtersCli6)
LINKBUNCH(${commonLibs2})
LINKBUNCH(${coreLibs})

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Add gtk specific libs
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
TARGET_LINK_LIBRARIES(avidemux3_cli PRIVATE
  ADM_UI_Cli6
  ADM_dialogCli6
  ADM_toolkit6
  ADM_coreAudio6
  ADM_coreUtils6
  ADM_gui2Cli6
  ADM_toolkitCli6
  ADM_shellCli
  ADM_render6
  ADM_coreVideoFilter6
)

# SDL
IF(USE_SDL)
  TARGET_LINK_LIBRARIES(avidemux3_cli PRIVATE ${SDL2_LIBRARY} ${SDL2_MAIN})
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# OS Specific
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
IF(WIN32 OR APPLE)
  SET_PROPERTY(TARGET avidemux3_cli PROPERTY OUTPUT_NAME avidemux_cli)
ENDIF()

IF(MINGW)
  TARGET_LINK_LIBRARIES(avidemux3_cli PRIVATE winmm -mwindows -Wl,-subsystem,console -Wl,--export-all-symbols)
ENDIF()

# Needed by RubberBand
IF(APPLE)
  #target_link_libraries(avidemux3_cli "-framework Accelerate")
  # Silence missing nullability specifier warnings on macOS
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-nullability-completeness")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nullability-completeness")
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Install
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
ADM_LINK_THREAD(avidemux3_cli)
ADM_INSTALL_BIN(avidemux3_cli)

DISPLAY_SUMMARY_LIST()

include(admPackager)
admPackager(cliPackage)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/cli/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_API_VERSION}/cli" COMPONENT dev)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/ADM_UIs/include/ADM_UI_Cli6_export.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_API_VERSION}/cli/ADM_UIs" COMPONENT dev)
MESSAGE("")

