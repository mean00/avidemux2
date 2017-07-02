#

cmake_minimum_required(VERSION 3.2)
MESSAGE(STATUS  "* Start *")
#
set(BU_CHMOD_BUNDLE_ITEMS ON)
SET(BU_COPY_FULL_FRAMEWORK_CONTENTS ON)
#
include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DeployQt5.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/admCheckQt5.cmake")
#
SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
SET(BUNDLE_NAME "Avidemux${AVIDEMUX_MAJOR_MINOR}.app")
SET(BUNDLE "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}")
SET(RESOURCES  "${BUNDLE}/Contents/Resources")
#

macro(makeLink src dest pwd)
  MESSAGE(STATUS " Creating symlink")
  MESSAGE( STATUS " COMMAND ln -sf ${src} ${dest} COMMENT mklink ${src} -> ${dest}, pwd=${pwd}")
  execute_process( COMMAND ln -sf ${src} ${dest} 
                   WORKING_DIRECTORY ${pwd})
endmacro()

MESSAGE(STATUS  "* Checking QT5 *")
#
checkQt5()
#
# Copy the skeleton, other files will be pulled automatically
#
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${BUNDLE_NAME})
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents)
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/MacOS)
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/Resources/)
# 
LIST(APPEND bundleDirs  "${BUNDLE}/Contents/Resources/lib")
#

#
MESSAGE(STATUS "* Creating Skeleton *")
FILE(COPY  "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/MacOS/Avidemux${AVIDEMUX_MAJOR_MINOR}" DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/MacOS/")
FILE(COPY  "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/Info.plist"                            DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/")
FILE(COPY  "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/Resources/lib"                       DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/Resources/")


makeLink("Resources/lib"  "." ${BUNDLE}/Contents/)
#
MESSAGE(STATUS "* Copying plugins *")
#
FILE(COPY  "${QT_PLUGINS_DIR}/platforms" DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/PlugIns/")
# Update Qt plugins
#
file(GLOB  pluginList "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/PlugIns/platforms/*.dylib")
MESSAGE(STATUS "List of plugins ${pluginList}")
FOREACH(plugin ${pluginList})
        MESSAGE(STATUS "=> ${plugin}")
        LIST(APPEND myPlugins ${plugin})
ENDFOREACH(plugin ${pluginList})
MESSAGE(STATUS ">>List of plugins to process ${myPlugins}")
MESSAGE(STATUS " * plugin *")
#
# Update avidemux plugins
# 
MESSAGE(STATUS "* ADM plugins *")
#
MESSAGE(STATUS " * ADMplugin *")
file(GLOB  kind "${BUNDLE}/Contents/lib/ADM_plugins6/*")
MESSAGE(STATUS "* -----<${kind}>-----  *")
FOREACH(k ${kind})
        MESSAGE(STATUS "* Processing ${k} *")
        file(GLOB_RECURSE  kind_${k} "${BUNDLE}/Contents/lib/ADM_plugins6/k/*.dylib")
        LIST(APPEND all_adm_plugin  ${kind_${k}})
ENDFOREACH(k ${kind})
#
LIST(APPEND all ${myPlugins})
LIST(APPEND all ${all_adm_plugin})
FIXUP_BUNDLE("${CMAKE_BINARY_DIR}/${BUNDLE_NAME}" "${all}" "${bundleDirs}")

# Now we can move the old libs
MESSAGE(STATUS " * Cleanup * ")
FILE(COPY "${BUNDLE}/Contents/lib/ADM_plugins6" DESTINATION "${BUNDLE}/Contents/MacOS/")
FILE(REMOVE_RECURSE  "${BUNDLE}/Contents/lib")
FILE(REMOVE_RECURSE  "${BUNDLE}/Contents/Resources/lib")
makeLink("MacOS"  "lib" ${BUNDLE}/Contents/)

MESSAGE(STATUS "* Last pass *")
#
FIXUP_BUNDLE("${CMAKE_BINARY_DIR}/${BUNDLE_NAME}" "${all_adm_plugin}" "${bundleDirs}")

# Create symlink
SET(BIN     "${BUNDLE}/Contents/MacOS/")
SET(ExeName "${BIN}/Avidemux${AVIDEMUX_MAJOR_MINOR}")
FILE(RENAME "${ExeName}"  "${BIN}/avidemux")
#
FILE(WRITE  "${ExeName}" "#!/bin/sh\n")
FILE(APPEND "${ExeName}" "DIR=`dirname $0`\n")
FILE(APPEND "${ExeName}" "export FONTCONFIG_FILE=$DIR/../Resources/fonts/fonts.conf\n")
FILE(APPEND "${ExeName}" "cd \"$DIR\" && ./avidemux \"$@\"")
execute_process( COMMAND chmod +x ${ExeName})

MESSAGE(STATUS "* Verifying *")
VERIFY_APP("${CMAKE_BINARY_DIR}/${BUNDLE_NAME}" )

# Qt plugin  => ko, adm_plugin => ko, Qt FRamework OK

MESSAGE(STATUS "* All done *")

