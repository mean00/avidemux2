MACRO(checkLibxml2)
	IF (NOT LIBXML2_CHECKED)
		MESSAGE(STATUS "Checking for Libxml2")
		MESSAGE(STATUS "********************")

		SET(LibXml2_FIND_QUIETLY TRUE)

                if(CROSS)
                        MESSAGE(STATUS "Xml2: Cross compilation override")
                        SET(WIN32 0)
                endif(CROSS) 

                FIND_PACKAGE(LibXml2)

                if(CROSS)
                        MESSAGE(STATUS "Cross : Xml2: ${LIBXML2_FOUND} lib:${LIBXML2_LIBRARIES} inc:${LIBXML2_INCLUDE_DIR} ")
                        SET(LIBXML2_LIBRARIES "-lxml2 -L${CROSS}/lib")
                        SET(WIN32 1)
                endif(CROSS) 

		PRINT_LIBRARY_INFO("Libxml2" LIBXML2_FOUND "${LIBXML2_INCLUDE_DIR} ${LIBXML2_DEFINITIONS}" "${LIBXML2_LIBRARIES}")

		SET(LIBXML2_CHECKED 1)

		MESSAGE("")
	ENDIF (NOT LIBXML2_CHECKED)
ENDMACRO(checkLibxml2)
