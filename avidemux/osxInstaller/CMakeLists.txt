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
# Copy jobs and cli too
FILE(COPY  "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/Resources/bin/avidemux_cli"          DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/MacOS/")
FILE(COPY  "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/Resources/bin/avidemux_jobs"         DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/MacOS/")
FILE(COPY  "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/Info.plist"                          DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}/Contents/")
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
FILE(COPY "${BUNDLE}/Contents/lib/ADM_plugins6" DESTINATION "${BUNDLE}/Contents/MacOS/")
file(GLOB  kind "${BUNDLE}/Contents/MacOS/ADM_plugins6/*")
MESSAGE(STATUS "* -----<${kind}>-----  *")
FOREACH(k ${kind})
        MESSAGE(STATUS "* Processing ${k} *")
        file(GLOB_RECURSE  kind_${k} "${k}/*.dylib")
        MESSAGE(STATUS "\t ${kind_${k}}")
        LIST(APPEND all_adm_plugin  ${kind_${k}})
ENDFOREACH(k ${kind})
#
LIST(APPEND all ${myPlugins})
LIST(APPEND all ${all_adm_plugin})
MESSAGE(STATUS "Plugin to fix : ${all}")
FIXUP_BUNDLE("${CMAKE_BINARY_DIR}/${BUNDLE_NAME}" "${all}" "${bundleDirs}")
# Now we can move the old libs
MESSAGE(STATUS " * Cleanup * ")
FILE(REMOVE_RECURSE  "${BUNDLE}/Contents/lib")
FILE(REMOVE_RECURSE  "${BUNDLE}/Contents/Resources/lib")
makeLink("MacOS"  "lib" ${BUNDLE}/Contents/)

MESSAGE(STATUS "*  Wrapper script *")
#

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

# Copy fonts, i18n and icons
FILE(COPY "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/Resources/share" DESTINATION "${RESOURCES}") 
FILE(COPY "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/Resources/fonts" DESTINATION "${RESOURCES}") 
FILE(COPY "${CMAKE_BINARY_DIR}/../${BUNDLE_NAME}/Contents/Resources/avidemux.icns" DESTINATION "${RESOURCES}") 

# Remove the copies of Qt lib in framework, we only keep "5"
FILE(GLOB  frameworks "${BUNDLE}/Contents/Frameworks/*")
MESSAGE(STATUS "Frameworks to clean : ${frameworks}")
FOREACH(frame ${frameworks})
        MESSAGE(STATUS "    Cleaning ${frame}")
        FILE(REMOVE_RECURSE  "${frame}/Headers")
        FILE(GLOB  qtop      "${frame}/Qt*")
        FOREACH(q ${qtop})
                MESSAGE(STATUS "  >${q}")
                FILE(REMOVE    "${q}")
        ENDFOREACH(q ${qtop})
        FILE(REMOVE_RECURSE  "${frame}/Versions/Current")
        FILE(REMOVE_RECURSE  "${frame}/Versions/5/Headers")
        # Create symlinks ?
ENDFOREACH(frame ${frameworks})

#
# Qt plugin  => ko, adm_plugin => ko, Qt FRamework OK
MESSAGE(STATUS "* Creating dmg *")
string(TIMESTAMP now "%Y%m%d")
execute_process( COMMAND hdiutil create Avidemux${AVIDEMUX_MAJOR_MINOR}_${now}.dmg -srcfolder ${CMAKE_BINARY_DIR}/Avidemux${AVIDEMUX_MAJOR_MINOR}.app/ -ov
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

MESSAGE(STATUS "* All done *")

