#
cmake_minimum_required(VERSION 3.2)
MESSAGE(STATUS  "* Start *")
#
set(BU_CHMOD_BUNDLE_ITEMS ON)
SET(BU_COPY_FULL_FRAMEWORK_CONTENTS ON)
#
include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/BundleUtilities.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DeployQt5.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/admCheckQt5.cmake")
#
SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
SET(BUNDLE_NAME "Avidemux${AVIDEMUX_MAJOR_MINOR}.app")
SET(BUNDLE "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}")
SET(RESOURCES  "${BUNDLE}/Contents/Resources")
#
MESSAGE(STATUS  "* Checking QT4 *")
#
checkQt5()
#
# Copy the skeleton, other files will be pulled automatically
#
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${BUNDLE_NAME})
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents)
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/MacOS)
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/Resources)
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/Resources/lib)
# 
MESSAGE(STATUS "* Creating Skeleton *")
FILE(COPY  "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/MacOS/Avidemux${AVIDEMUX_MAJOR_MINOR}" DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/MacOS/")
FILE(COPY  "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/Info.plist"                            DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/")
FILE(COPY  "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/Resources/lib"                       DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/")

#
MESSAGE(STATUS "* Copying plugins *")
#
FILE(COPY  "/opt/local/libexec/qt5/plugins/platforms" DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/PlugIns/")
# Update Qt plugins
file(GLOB  pluginlist "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/PlugIns/platforms/*.dylib")
MESSAGE(STATUS "List of plugins ${pluginlist}")
FOREACH(plugin ${pluginList})
        MESSAGE(STATUS "=> ${plugin}")
        LIST(APPEND plugins ${plugin})
ENDFOREACH(plugin ${pluginList})
MESSAGE(STATUS "List of plugins ${plugins}")

#
#fixup_qt5_executable("${CMAKE_BINARY_DIR}/${BUNDLE_NAME}" "" "" "")#"${RESOURCES}/bin")
#install_qt5_executable("${CMAKE_BINARY_DIR}/${BUNDLE_NAME}" "" )#"${RESOURCES}/bin")
FIXUP_BUNDLE("${CMAKE_BINARY_DIR}/${BUNDLE_NAME}" "${pluginList}" "")

# Now we can remove the old libs
MESSAGE(STATUS "Cleanup")
file(GLOB  oldLibs "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/lib/libADM*.dylib")
FILE(REMOVE ${oldLibs})

MESSAGE(STATUS "* Verifying *")
VERIFY_APP("${CMAKE_BINARY_DIR}/${BUNDLE_NAME}" )

# Qt plugin  => ko, adm_plugin => ko, Qt FRamework OK

MESSAGE(STATUS "* All done *")

