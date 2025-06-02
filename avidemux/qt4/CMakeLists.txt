CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for MSVC we use vcpk, it is complicated way of detecting it
# because we have to do it before PROJECT(...)
IF(EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  SET(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  MESSAGE(STATUS "Using MSVC VCPKG setup <${CMAKE_TOOLCHAIN_FILE}>")
ENDIF()


PROJECT(Avidemux_Qt)
SET(UI_TYPE ADM_UI_QT4)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
MESSAGE("")
MESSAGE("#########################################")
MESSAGE("Configure for Avidemux Qt Started")
MESSAGE("#########################################")
MESSAGE("")
MESSAGE(STATUS "Checking for avidemux development files ..")

ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_QT4)

IF(MSVC)
  MESSAGE(STATUS "Temporary hack")
  ADD_COMPILE_OPTIONS(/std:c++17 /Zc:__cplusplus  /permissive- )
ENDIF()

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
IF(NOT FAKEROOT)
  SET(AVIDEMUX_FAKEROOT "")
ELSE()
  SET(AVIDEMUX_FAKEROOT "${FAKEROOT}")
ENDIF()
INCLUDE(avidemux_qt_api)
SET(ADM_HEADER_DIR ${AVIDEMUX_FAKEROOT}${CMAKE_INSTALL_PREFIX}/include/avidemux/${AVIDEMUX_API_VERSION})
SET(ADM_CMAKE_DIR  ${ADM_HEADER_DIR}/cmake)
LIST(APPEND CMAKE_MODULE_PATH ${ADM_CMAKE_DIR})
#
# Sanity check
#
MESSAGE(STATUS  "Checking if avidemux include folder exists as ${ADM_HEADER_DIR}")
MESSAGE(STATUS  "(CMAKE_INSTALL_PREFIX/include/avidemux/${AVIDEMUX_API_VERSION})")
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
MESSAGE(STATUS "Found cmake subfolder. good.")

#
# Go
#
INCLUDE(avidemuxVersion)
INCLUDE(admFFmpegVersion)
MESSAGE(STATUS "Building agains version ${AVIDEMUX_VERSION} with fflibs ${FFMPEG_VERSION}")

IF(WIN32)
  IF(MSVC)
    INCLUDE(adm_win32_vs)
  ELSE()
    INCLUDE(adm_win32_mingw)
  ENDIF()
ELSE()
  IF(APPLE)
    # Silence missing nullability specifier warnings on macOS
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-nullability-completeness")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nullability-completeness")
    INCLUDE(adm_osx_qt)
  ELSE()
    INCLUDE(adm_default)
  ENDIF()
ENDIF()
#-----------------------------------
# Set extra libs, system dependant
#-----------------------------------
ADM_PREBUILD()

# Locate commonCmakeApplication.cmake
INCLUDE(commonCmakeApplication)
INCLUDE(../admAppSettings.cmake)
INCLUDE(admCheckQt)
INCLUDE(admWindRes)

MESSAGE(STATUS "Checking Qt")
checkQt()

if(NOT QT_FOUND)
  MESSAGE(FATAL_ERROR "Qt NOT FOUND")
ENDIF()

# Qt4 openGL
INCLUDE(admCheckOpenGl)
IF(NOT APPLE)
  INCLUDE(FindSDL2)
ENDIF()
#
#--
MESSAGE(STATUS "Adding Qt inc paths : <${QT_INCLUDES}> and definitions <${QT_DEFINITIONS}>")
INCLUDE_DIRECTORIES(${QT_INCLUDES})
ADD_DEFINITIONS(${QT_DEFINITIONS})
#--

INCLUDE(admCheckFreeType)
checkFreeType()
INCLUDE(admCheckFontConfig)
checkFontConfig()

#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Config
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
SET(CONFIG_HEADER_TYPE ADM_BUILD_QT4)
SET(UI_SUFFIX ${QT_EXTENSION})


CONFIGURE_FILE("${ADM_CMAKE_DIR}/config.h.cmake" "${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/config.h")
MESSAGE(STATUS "${QT_EXTENSION} config.h generated")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/")
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../common)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ADM_render_qt)

ADD_SUBDIRECTORY(ADM_render_qt)
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Add job control
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
ADD_SUBDIRECTORY(ADM_jobs)
ADD_SUBDIRECTORY(ADM_update)
IF(USE_OPENGL)
  ADD_SUBDIRECTORY(ADM_openGL)
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Custom fixed width font for time display
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
IF(NOT WIN32)
  SET(USE_CUSTOM_TIME_DISPLAY_FONT 1)
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Add common as a symlink or directly
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
INCLUDE_DIRECTORIES(ADM_UIs/include/)

IF(WIN32 AND NOT CROSS)
  ADD_SUBDIRECTORY(../common ./commonQt4)
ELSE()
  # Make symlink else eclipe and kdev4 are puzzled by the tree structure
  # Not needed for plain build
  MESSAGE(STATUS "Creating common symlink in ${CMAKE_CURRENT_SOURCE_DIR}")

  EXECUTE_PROCESS(COMMAND rm -f common
                                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  EXECUTE_PROCESS(COMMAND ln -s ../common .
                                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  ADD_SUBDIRECTORY(common ./commonQt4)
ENDIF()

# Add misc subdirs
ADD_SUBDIRECTORY(ADM_commonUI)
ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsQt4)
ADD_SUBDIRECTORY(ADM_qt ./ADM_qtxQt4)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfaces${QT_LIBRARY_EXTENSION})

SDLify(../common/main.cpp)
IF(ADM_SUBVERSION)
  ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_SUBVERSION=\"${ADM_SUBVERSION}\"")
  ADD_SOURCE_CFLAGS(../common/main.cpp "-DFFMPEG_VERSION=\"${FFMPEG_VERSION}\"")
ENDIF()
ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_VERSION=\"${AVIDEMUX_VERSION}\"")
ADD_SOURCE_CFLAGS(../common/main.cpp "-DQT_FLAVOR=\"${QT_EXTENSION}\"")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Icon for windows
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
IF(WIN32)
  WINDRESIFY(mainQt  "${CMAKE_CURRENT_SOURCE_DIR}/../common/xpm/adm.ico" src "avidemux" "Avidemux Video Editor")
  SET(ADM_EXE_SRCS ${ADM_EXE_SRCS} ${src})
ENDIF()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Executable
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

ADM_MAIN_APP()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Construct common libraries
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
FOREACH(_libName ${commonLibs1})
  TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} PRIVATE ${_libName})
ENDFOREACH()

FOREACH(_libName ${commonLibs2})
  TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} PRIVATE ${_libName})
ENDFOREACH()

FOREACH(_libName ${coreLibs})
  TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} PRIVATE ${_libName})
ENDFOREACH()



# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Add qt specific libs
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} PRIVATE
   ADM_gui${QT_LIBRARY_EXTENSION}
   ADM_filters${QT_LIBRARY_EXTENSION}
   ADM_UI${QT_LIBRARY_EXTENSION}6
   ADM_dialogQt4
   ADM_update${QT_LIBRARY_EXTENSION}6
   ADM_internalVideoFilter6
   ADM_UI${QT_LIBRARY_EXTENSION}6
   ADM_gui${QT_LIBRARY_EXTENSION}
   ADM_UI_${QT_LIBRARY_EXTENSION}6
   ADM_shell${QT_LIBRARY_EXTENSION}
   ADM_editor6_interface
   ADM_toolkit6
   ADM_coreVideoFilter6
   ADM_internalVideoFilter6
   ADM_coreAudio6
   ADM_coreAudioDevice6
   ADM_coreVideoEncoder6
   ADM_osSupport6
   adm_pthread
)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# External libs
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# gettext
TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} PRIVATE adm_gettext )


# SDL
if(USE_SDL)
  TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} PRIVATE ${SDL2_LIBRARY} ${SDL2_MAIN})
ENDIF()


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# UI Specific
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} PRIVATE ADM_qtx_${QT_EXTENSION}  ADM_render6 ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} PRIVATE ${PlatformLibs})
TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} PRIVATE ADM_commonUI6_${QT_EXTENSION})

# install qlementine theme
#IF("${QT_EXTENSION}" STREQUAL "qt6")
  #ADD_SUBDIRECTORY(qlementine)
  #TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} PRIVATE oclero::qlementine)
#ENDIF()
#

#
# i18n
#
ADD_SUBDIRECTORY(i18n)
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Install
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

DISPLAY_SUMMARY_LIST()

# install XDG metadata
IF(UNIX AND NOT APPLE)
  ADD_SUBDIRECTORY(xdg_data)
ENDIF()

ADM_POSTBUILD()
INCLUDE(admPackager)
admPackager(qt4Package)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/config.h DESTINATION "${AVIDEMUX_INSTALL_INCLUDE_DIR}/avidemux/${AVIDEMUX_API_VERSION}/${QT_EXTENSION}" COMPONENT dev)

# Install QT stuff
ADM_INSTALL_CMAKE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# install headers
ADM_INSTALL_QT_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/ADM_openGL/include/"  ADM_openGL)
ADM_INSTALL_QT_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/ADM_UIs/include/"  ADM_UIs)

MESSAGE("")
