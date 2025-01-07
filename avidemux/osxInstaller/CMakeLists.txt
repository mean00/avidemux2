CMAKE_MINIMUM_REQUIRED(VERSION 3.7)
PROJECT("AvidemuxBundle")
MESSAGE(STATUS  "* Start *")

SET(ORIGINAL "${CMAKE_BINARY_DIR}/../Avidemux${AVIDEMUX_MAJOR_MINOR}.app")
SET(ADM_CMAKE_DIR "${ORIGINAL}/Contents/Resources/include/avidemux/${AVIDEMUX_MAJOR_MINOR}/cmake") # needed by config helper
IF(NOT FAKEROOT)
  SET(AVIDEMUX_FAKEROOT "")
ELSE()
  SET(AVIDEMUX_FAKEROOT "${FAKEROOT}")
ENDIF()
# To get icons etc..
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_CURRENT_SOURCE_DIR}/../qt4/cmake )
#probably not needed
LIST(APPEND CMAKE_MODULE_PATH ${AVIDEMUX_FAKEROOT}${CMAKE_INSTALL_PREFIX}/include/avidemux/${AVIDEMUX_API_VERSION}/cmake ${ADM_CMAKE_DIR})
#
# Can we instead propagate vars set during the build?
INCLUDE(admConfigHelper)
INCLUDE(admDetermineSystem)

INCLUDE(admCheckQt)

IF(ENABLE_QT6)
  MESSAGE(STATUS  "* Checking Qt6 *")
ELSE()
  IF(ENABLE_QT5)
    MESSAGE(STATUS "* Checking Qt5 *")
  ELSE()
    MESSAGE(FATAL_ERROR "Unsupported Qt version requested")
  ENDIF()
ENDIF()

checkQt()

IF(NOT QT_FOUND)
  MESSAGE(FATAL_ERROR "Qt not found")
ENDIF()

SET(BUNDLE_NAME "Avidemux-${AVIDEMUX_VERSION}.app")
SET(BUNDLE "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}")
SET(RESOURCES  "${BUNDLE}/Contents/Resources")
#
# Copy the skeleton, other files will be pulled automatically
#
FILE(MAKE_DIRECTORY "${BUNDLE}")
FILE(MAKE_DIRECTORY "${BUNDLE}/Contents")
FILE(MAKE_DIRECTORY "${BUNDLE}/Contents/MacOS")
FILE(MAKE_DIRECTORY "${BUNDLE}/Contents/Resources")
#
LIST(APPEND bundleDirs  "${BUNDLE}/Contents/Resources/lib")
IF(ENABLE_QT6)
  # A gross hack to work around the failure to resolve @loader_path for libbrotli.
  LIST(APPEND bundleDirs "/usr/local/lib")
ENDIF()
#

#
MESSAGE(STATUS "* Creating Skeleton *")
# Quick Hack...
#FILE(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../qt4/ADM_userInterfaces/ADM_gui/pics/avidemux.icns"
#  DESTINATION "${ORIGINAL}/Contents/Resources/avidemux.icns")
FILE(COPY "${ORIGINAL}/Contents/MacOS/Avidemux${AVIDEMUX_MAJOR_MINOR}" DESTINATION "${BUNDLE}/Contents/MacOS/")
# Copy jobs and cli too
FILE(COPY "${ORIGINAL}/Contents/Resources/bin/avidemux_cli"          DESTINATION "${BUNDLE}/Contents/MacOS/")
FILE(COPY "${ORIGINAL}/Contents/Resources/bin/avidemux_jobs"         DESTINATION "${BUNDLE}/Contents/MacOS/")
FILE(COPY "${ORIGINAL}/Contents/Info.plist"                          DESTINATION "${BUNDLE}/Contents/")
FILE(COPY "${ORIGINAL}/Contents/Resources/lib"                       DESTINATION "${BUNDLE}/Contents/Resources/")

INCLUDE(linkMacro)

makeLink("Resources/lib"  "." ${BUNDLE}/Contents/)
#
MESSAGE(STATUS "* Copying plugins *")
#
FILE(COPY "${QT_PLUGINS_DIR}/platforms" DESTINATION "${BUNDLE}/Contents/PlugIns/" DIRECTORY_PERMISSIONS
    OWNER_WRITE OWNER_READ OWNER_EXECUTE
    GROUP_WRITE GROUP_READ GROUP_EXECUTE
    WORLD_WRITE WORLD_READ WORLD_EXECUTE)
IF(EXISTS "${QT_PLUGINS_DIR}/styles")
  FILE(COPY "${QT_PLUGINS_DIR}/styles" DESTINATION "${BUNDLE}/Contents/PlugIns/" DIRECTORY_PERMISSIONS
        OWNER_WRITE OWNER_READ OWNER_EXECUTE
        GROUP_WRITE GROUP_READ GROUP_EXECUTE
        WORLD_WRITE WORLD_READ WORLD_EXECUTE)
ENDIF()
# Update Qt plugins
#
FILE(GLOB  pluginList "${BUNDLE}/Contents/PlugIns/platforms/*.dylib" "${BUNDLE}/Contents/PlugIns/styles/*.dylib")
MESSAGE(STATUS "List of plugins ${pluginList}")
FOREACH(plugin ${pluginList})
  MESSAGE(STATUS "=> ${plugin}")
  LIST(APPEND myPlugins ${plugin})
ENDFOREACH()
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
ENDFOREACH()
#
LIST(APPEND all ${myPlugins})
LIST(APPEND all ${all_adm_plugin})
MESSAGE(STATUS "Plugin to fix : ${all}")

# Create Qt.conf
SET(QT_CONF "${BUNDLE}/Contents/Resources/qt.conf")
FILE(WRITE  "${QT_CONF}" "[Paths]\n")
FILE(APPEND "${QT_CONF}" "Plugins = PlugIns\n")

# Copy fonts, i18n and icons
FILE(COPY "${ORIGINAL}/Contents/Resources/share" DESTINATION "${RESOURCES}")
FILE(COPY "${ORIGINAL}/Contents/Resources/fonts" DESTINATION "${RESOURCES}")
FILE(COPY "${ORIGINAL}/Contents/Resources/avidemux.icns" DESTINATION "${RESOURCES}")

# Entice macOS to return correct system language (QTBUG-72491)
FILE(GLOB ListOfTranslationFiles "${RESOURCES}/share/avidemux6/qt${ADM_QT_VERSION}/i18n/avidemux_*.qm")
FOREACH(lang ${ListOfTranslationFiles})
  STRING(REGEX REPLACE ".*avidemux_" "" lang "${lang}")
  STRING(REPLACE ".qm" "" lang "${lang}")
  MESSAGE(STATUS "Creating directory ${BUNDLE}/Contents/Resources/${lang}.lproj")
  FILE(MAKE_DIRECTORY "${BUNDLE}/Contents/Resources/${lang}.lproj")
ENDFOREACH()

# Fixup bundle
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/admFixupBundle.cmake.in" "${CMAKE_BINARY_DIR}/admFixupBundle.cmake" @ONLY)
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake/macos-adhoc-sign-app.sh.in" "${CMAKE_BINARY_DIR}/macos-adhoc-sign-app.sh" @ONLY)
INSTALL(SCRIPT "${CMAKE_BINARY_DIR}/admFixupBundle.cmake" COMPONENT Runtime)

#
MESSAGE(STATUS "* Creating dmg *")
# Call cpack to create the dmg using the dragndrop installer
IF(NOT DMG_BASENAME)
  SET(DMG_BASENAME "Avidemux ${AVIDEMUX_VERSION}")
  IF(ENABLE_QT6)
    SET(DMG_BASENAME "${DMG_BASENAME} Qt6")
  ELSE()
    IF(ENABLE_QT5)
      SET(DMG_BASENAME "${DMG_BASENAME} Qt5")
    ENDIF()
  ENDIF()
  IF(NOT RELEASE AND BUILD_REV)
    SET(DMG_BASENAME "${DMG_BASENAME} ${BUILD_REV}")
  ENDIF()
ENDIF()

#
SET(CPACK_GENERATOR         "DragNDrop")
SET(CPACK_PACKAGE_NAME      "Avidemux")
SET(CPACK_PACKAGE_VENDOR    "Mean")
SET(CPACK_SOURCE_GENERATOR  "")
SET(CPACK_PACKAGE_FILE_NAME "${DMG_BASENAME}")
SET(CPACK_DMG_FORMAT        "UDBZ")
SET(CPACK_DMG_VOLUME_NAME   "${CPACK_PACKAGE_FILE_NAME}")
SET(CPACK_BINARY_DRAGNDROP  ON)
SET(CPACK_PACKAGE_ICON      "${CMAKE_SOURCE_DIR}/avidemux_dmg.icns")

SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
INSTALL(DIRECTORY "${BUNDLE}" DESTINATION . COMPONENT Runtime USE_SOURCE_PERMISSIONS)

INCLUDE(CPack)

MESSAGE(STATUS "* All done *")

