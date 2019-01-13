cmake_minimum_required(VERSION 3.7)
project(QTIFW)
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake" "${CMAKE_MODULE_PATH}")
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/avidemuxVersion.cmake)
SET(ADM_VERSION "${AVIDEMUX_VERSION}")
SET(ADM_MENU_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(ADM_FLAVOR "64bits")
    SET(ApplicationsDirX64 "@ApplicationsDirX64@")
ELSE (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(ADM_FLAVOR "32bits")
    SET(ApplicationsDirX64 "@ApplicationsDir@")
ENDIF (CMAKE_SIZEOF_VOID_P EQUAL 8)

IF(MSVC)
    SET(ADM_FLAVOR "VC++ ${ADM_FLAVOR}")
ELSE(MSVC)
ENDIF(MSVC)

IF(NOT RELEASE)
    ADM_TIMESTAMP(date)
    SET(ADM_EXTRA "${date}")
ENDIF(NOT RELEASE)

MESSAGE(STATUS "Creating installer framework for ${ADM_FLAVOR}, version ${ADM_VERSION}")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config/config.xml.in ${CMAKE_BINARY_DIR}/config/config.xml @ONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config/watermark.png ${CMAKE_BINARY_DIR}/config/watermark.png  COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config/logo.png ${CMAKE_BINARY_DIR}/config/logo.png  COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config/avidemux.ico ${CMAKE_BINARY_DIR}/config/avidemux.ico  COPYONLY)
#
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/packages/org.avidemux.run/meta/package.xml.in ${CMAKE_BINARY_DIR}/packages/org.avidemux.run/meta/package.xml @ONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/packages/org.avidemux.dev/meta/package.xml.in ${CMAKE_BINARY_DIR}/packages/org.avidemux.dev/meta/package.xml @ONLY)
#
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/packages/org.avidemux.run/meta/license.txt ${CMAKE_BINARY_DIR}/packages/org.avidemux.run/meta/license.txt  COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/packages/org.avidemux.run/meta/installscript.js  ${CMAKE_BINARY_DIR}/packages/org.avidemux.run/meta/installscript.js   COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/packages/org.avidemux.run/meta/targetwidget.ui  ${CMAKE_BINARY_DIR}/packages/org.avidemux.run/meta/targetwidget.ui   COPYONLY)
#
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/packages/org.avidemux.dev/meta/installscript.qs ${CMAKE_BINARY_DIR}/packages/org.avidemux.dev/meta/installscript.qs  COPYONLY)

#
IF(MSVC)
        SET(PREFIX_CMD "binarycreator.exe")
        SET(ACTUAL_CMD "")
ELSE(MSVC)
        SET(PREFIX_CMD "wine64")
        SET(ACTUAL_CMD "/opt/QtIFW-3.0.6/bin/binarycreator.exe")
ENDIF(MSVC)

INSTALL(CODE "MESSAGE(STATUS \"Creating qt installer\")"
        CODE "execute_process (COMMAND ${PREFIX_CMD} ${ACTUAL_CMD} -c config/config.xml  -p packages \"Avidemux_${ADM_VERSION} ${ADM_FLAVOR} ${ADM_EXTRA}\"
                            RESULT_VARIABLE result ERROR_VARIABLE error_var)"
    )
