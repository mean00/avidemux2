        IF(CROSS)
                SET(WINDRES ${CMAKE_RC_COMPILER})
        ELSE(CROSS)
                SET(WINDRES windres.exe)
        ENDIF(CROSS)
include(admTimeStamp)
#
MACRO(WINDRESIFY tag icon src)

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
        

        IF(WIN32)
            include(generate_product_version)
            generate_product_version(ProductVersionFiles_${tag}
                    NAME avidemux3
                    VERSION_MAJOR ${CPACK_PACKAGE_VERSION_MAJOR}
                    VERSION_MINOR ${CPACK_PACKAGE_VERSION_MINOR}
                    VERSION_PATCH ${CPACK_PACKAGE_VERSION_P}
                    ICON          ${ICON_PATH}
                    VERSION_REVISION 30303
                    COMPANY_NAME avidemux.org)
           MESSAGE(STATUS "RC file info : ${${src}}")
           # In case of mingw we need to conver the .res to .o
           IF(MSVC)
            SET( ${src} ${ProductVersionFiles_${tag}})
           ELSE(MSVC)
            SET( ${src}  ${CMAKE_CURRENT_BINARY_DIR}/rcFile.o)
            ADD_CUSTOM_COMMAND(OUTPUT ${src} COMMAND ${WINDRES} -F ${WIN_RES_TARGET} -i  ${ProductVersionFiles_${tag}}  -o ${${src}} -O coff --define VS_VERSION_INFO=1)
           ENDIF(MSVC)

           
        ENDIF (WIN32)
ENDMACRO(WINDRESIFY tag icon src)
