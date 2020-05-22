        IF(CROSS)
                SET(WINDRES ${CMAKE_RC_COMPILER})
        ELSE(CROSS)
                SET(WINDRES windres.exe)
        ENDIF(CROSS)
#
MACRO(WINDRESIFY tag icon src basename desc)

        IF (ADM_CPU_X86_64)
            SET(WIN_RES_TARGET "pe-x86-64")
        ELSE (ADM_CPU_X86_64)
            SET(WIN_RES_TARGET "pe-i386")
        ENDIF (ADM_CPU_X86_64)

        # Convert to native absolute path
        SET(FULL_PATH "${icon}")
        get_filename_component(abs "${FULL_PATH}" ABSOLUTE)
        file(TO_NATIVE_PATH ${abs} ICON_PATH)
        # replace c:\foo by c:\\foo
        STRING(REPLACE "\\" "\\\\" ICON_PATH ${ICON_PATH})
        IF(${CMAKE_VERSION} VERSION_GREATER 2.8.11)
                STRING(TIMESTAMP CURRENT_YEAR "%Y")
                STRING(TIMESTAMP BUILD_ID "%y%j")
        ELSE()
                STRING(TIMESTAMP CURRENT_YEAR "2020")
                STRING(TIMESTAMP BUILD_ID "0")
        ENDIF()
        SET(COPYRIGHT_STRING "Copyright © 2001-${CURRENT_YEAR} Avidemux Devs")
        SET(EXECUTABLE_DESCRIPTION "${desc}")
        SET(EXECUTABLE_FILENAME "${basename}.exe")
        SET(EXECUTABLE_REVISION "${BUILD_ID}")
        IF(MSVC)
                include(generate_product_version)
                generate_product_version(ProductVersionFiles_${tag}
                        NAME "${EXECUTABLE_DESCRIPTION}"
                        BUNDLE Avidemux
                        VERSION_MAJOR ${CPACK_PACKAGE_VERSION_MAJOR}
                        VERSION_MINOR ${CPACK_PACKAGE_VERSION_MINOR}
                        VERSION_PATCH ${CPACK_PACKAGE_VERSION_P}
                        ICON          ${ICON_PATH}
                        VERSION_REVISION ${EXECUTABLE_REVISION}
                        COMPANY_NAME avidemux.org
                        COMPANY_COPYRIGHT "${COPYRIGHT_STRING}"
                        ORIGINAL_FILENAME "${EXECUTABLE_FILENAME}"
                        INTERNAL_NAME avidemux)
                MESSAGE(STATUS "RC file info : ${${src}}")
                SET( ${src} ${ProductVersionFiles_${tag}})
         ELSE() # MINGW
                # add icon and version info
                SET(FILEVERSION_STRING "${AVIDEMUX_VERSION}")
                SET(PRODUCTVERSION_STRING "${AVIDEMUX_VERSION}")
                STRING(REPLACE "." "," FILEVERSION ${FILEVERSION_STRING})
                STRING(REPLACE "." "," PRODUCTVERSION ${PRODUCTVERSION_STRING})
                SET(PRODUCTVERSION "${PRODUCTVERSION},${EXECUTABLE_REVISION}")
                SET(FILEVERSION "${FILEVERSION},${EXECUTABLE_REVISION}")

                CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/admWin32.rc.in  ${CMAKE_CURRENT_BINARY_DIR}/admWin.rc IMMEDIATE)
                SET(ADM_WIN_RES "adm.obj")
                SET( ${src} ${ADM_WIN_RES})
                ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ADM_WIN_RES} COMMAND ${WINDRES} -F ${WIN_RES_TARGET} -i ${CMAKE_CURRENT_BINARY_DIR}/admWin.rc -o ${CMAKE_CURRENT_BINARY_DIR}/${ADM_WIN_RES} -O coff --define VS_VERSION_INFO=1)
        ENDIF() # MINGW

ENDMACRO(WINDRESIFY tag icon src basename desc)
