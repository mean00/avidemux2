#
cmake_minimum_required(VERSION 3.2)
SET(BUNDLE "${CMAKE_BINARY_DIR}/Avidemux${AVIDEMUX_MAJOR_MINOR}.app")
include(BundleUtilities)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DeployQt5.cmake")

#  Duplicate the folder
MESSAGE(STATUS "* Duplicating bundle *")
FILE(COPY  "${CMAKE_BINARY_DIR}/../Avidemux${AVIDEMUX_MAJOR_MINOR}.app" DESTINATION ".")
MESSAGE(STATUS "* Renaming libs *")
file(RENAME "${BUNDLE}/Contents/Resources/lib"  "${BUNDLE}/Contents/lib")
IF(FALSE)
MESSAGE(STATUS "* Fixing  exe *")
file(RENAME "${BUNDLE}/Contents/MacOS/Avidemux${AVIDEMUX_MAJOR_MINOR}" "${BUNDLE}/Contents/Resources/bin/Avidemux${AVIDEMUX_MAJOR_MINOR}")
# Create wrapper script
#FILE(WRITE  "${BUNDLE}/Contents/MacOS/Avidemux${AVIDEMUX_MAJOR_MINOR}" "#!/bin/sh\n")
#FILE(APPEND "${BUNDLE}/Contents/MacOS/Avidemux${AVIDEMUX_MAJOR_MINOR}" "DIR=`dirname $0`\n")
#FILE(APPEND "${BUNDLE}/Contents/MacOS/Avidemux${AVIDEMUX_MAJOR_MINOR}" "export FONTCONFIG_FILE=$DIR/../Resources/fonts/fonts.conf\n")
#FILE(APPEND "${BUNDLE}/Contents/MacOS/Avidemux${AVIDEMUX_MAJOR_MINOR}" "cd \"$DIR/../Resources/bin/\" && ./Avidemux${AVIDEMUX_MAJOR_MINOR} \"$@\"\n")
FILE(COPY "${CMAKE_BINARY_DIR}/hi" "${BUNDLE}/Contents/MacOS/Avidemux${AVIDEMUX_MAJOR_MINOR}")
ENDIF(FALSE)
# 
SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")

install_qt5_executable("Avidemux${AVIDEMUX_MAJOR_MINOR}.app" "qcocoa" "Contents/lib")



