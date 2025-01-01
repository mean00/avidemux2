CMAKE_MINIMUM_REQUIRED(VERSION 3.7)
IF(MSVC)
  IF(EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    SET(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    MESSAGE(STATUS "Using MSVC VCPKG setup <${CMAKE_TOOLCHAIN_FILE}>")
  ELSE()
    MESSAGE(FATAL_ERROR "VCPKG not setup properly")
  ENDIF()
ENDIF()
#
#
#
PROJECT(QTIFW)
#
IF(MSVC)
  SET(VCPKG_BIN_DIR "$ENV{VCPKG_ROOT}/installed/x64-windows/bin")
  # Copy vcpkg runtime dll
  FILE(GLOB dlls   ${VCPKG_BIN_DIR}/*.dll)
  MESSAGE(STATUS "Copying VCPKG files")
  INSTALL(FILES ${dlls} DESTINATION ${CMAKE_BINARY_DIR}/packages/org.avidemux.run/data)
ENDIF()
INCLUDE(../../qt4/cmake/avidemux_qt_api.cmake)

SET(ADM_HEADER_DIR ${AVIDEMUX_FAKEROOT}${CMAKE_INSTALL_PREFIX}/include/avidemux/${AVIDEMUX_API_VERSION})
SET(ADM_CMAKE_DIR  ${ADM_HEADER_DIR}/cmake)
LIST(APPEND CMAKE_MODULE_PATH ${ADM_CMAKE_DIR})
#
INCLUDE(avidemuxVersion)
SET(ADM_VERSION "${AVIDEMUX_VERSION}")
SET(ADM_MENU_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(ADM_FLAVOR "64bits")
  SET(ApplicationsDirX64 "@ApplicationsDirX64@")
ELSE()
  SET(ADM_FLAVOR "32bits")
  SET(ApplicationsDirX64 "@ApplicationsDir@")
ENDIF()

IF(MSVC)
  SET(ADM_FLAVOR "VC++ ${ADM_FLAVOR}")
ELSE()
ENDIF()

IF(NOT RELEASE)
  ADM_TIMESTAMP(date)
  SET(ADM_EXTRA "${date}")
ENDIF()

MESSAGE(STATUS "Creating installer framework for ${ADM_FLAVOR}, version ${ADM_VERSION}")

SET(META ${CMAKE_CURRENT_SOURCE_DIR}/packages/org.avidemux.run/meta/)
SET(META_DEV ${CMAKE_CURRENT_SOURCE_DIR}/packages/org.avidemux.dev/meta/)

SET(PKG ${CMAKE_BINARY_DIR}/packages/org.avidemux)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config/config.xml.in ${CMAKE_BINARY_DIR}/config/config.xml @ONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config/watermark.png ${CMAKE_BINARY_DIR}/config/watermark.png  COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config/logo.png ${CMAKE_BINARY_DIR}/config/logo.png  COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config/avidemux.ico ${CMAKE_BINARY_DIR}/config/avidemux.ico  COPYONLY)
#
CONFIGURE_FILE(${META}/package.xml.in   ${PKG}.run/meta/package.xml @ONLY)
CONFIGURE_FILE(${META}/license.txt      ${PKG}.run/meta/license.txt  COPYONLY)
CONFIGURE_FILE(${META}/installscript.js ${PKG}.run/meta/installscript.js   COPYONLY)
CONFIGURE_FILE(${META}/targetwidget.ui  ${PKG}.run/meta/targetwidget.ui   COPYONLY)
CONFIGURE_FILE(${META_DEV}/installscript.qs ${PKG}.dev/meta/installscript.qs  COPYONLY)
CONFIGURE_FILE(${META_DEV}/package.xml.in   ${PKG}.dev/meta/package.xml @ONLY)
#
IF(MSVC)
  SET(PREFIX_CMD "binarycreator.exe")
  SET(ACTUAL_CMD "")
ELSE()
  SET(PREFIX_CMD "wine64")
  SET(ACTUAL_CMD "/opt/QtIFW-3.0.6/bin/binarycreator.exe")
ENDIF()
MESSAGE(STATUS "Exec CODE execute_process (COMMAND ${PREFIX_CMD} ${ACTUAL_CMD} -c config/config.xml  -p packages Avidemux_${ADM_VERSION} ${ADM_FLAVOR} ${ADM_EXTRA}RESULT_VARIABLE result ERROR_VARIABLE error_var)")

INSTALL(CODE "MESSAGE(STATUS \"Creating qt installer\")"
        CODE "execute_process (COMMAND ${PREFIX_CMD} ${ACTUAL_CMD} -c config/config.xml  -p packages \"Avidemux_${ADM_VERSION} ${ADM_FLAVOR} ${ADM_EXTRA}\" RESULT_VARIABLE result ERROR_VARIABLE error_var)"
    )
MESSAGE(STATUS "Output <${result}> Error  <${error_var}>")
