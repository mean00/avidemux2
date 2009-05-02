# a small macro to create qm files out of ts's

MACRO(FIND_XSLTPROC)
    IF(NOT XSLTPROC_EXECUTABLE AND NOT XSLTPROC_NOT_FOUND)
        SET(XSLTPROC_NAME "xsltproc")
        FIND_PROGRAM(XSLTPROC_EXECUTABLE ${XSLTPROC_NAME})
        MARK_AS_ADVANCED(XSLTPROC_EXECUTABLE)

        IF (NOT XSLTPROC_EXECUTABLE)
          MESSAGE(STATUS "WARNING: ${XSLTPROC_NAME} not found - ts files can't be processed")
          SET(XSLTPROC_NOT_FOUND "1")     # to avoid double checking in one cmake run
        ENDIF (NOT XSLTPROC_EXECUTABLE)
    ENDIF(NOT XSLTPROC_EXECUTABLE AND NOT XSLTPROC_NOT_FOUND)
ENDMACRO(FIND_XSLTPROC)

MACRO(FIND_LRELEASE)
    IF(NOT LRELEASE_EXECUTABLE AND NOT LRELEASE_NOT_FOUND)
		FIND_PROGRAM(LRELEASE_EXECUTABLE lrelease PATHS
			"[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
			"[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
			$ENV{QTDIR}/bin)

		IF (NOT LRELEASE_EXECUTABLE)
			MESSAGE(FATAL_ERROR "${LRELEASE_EXECUTABLE} not found - ts files can't be processed")
			SET(LRELEASE_NOT_FOUND "1")     # to avoid double checking in one cmake run
		ENDIF (NOT LRELEASE_EXECUTABLE)

	ENDIF(NOT LRELEASE_EXECUTABLE AND NOT LRELEASE_NOT_FOUND)
ENDMACRO(FIND_LRELEASE)

MACRO(COMPILE_AVIDEMUX_TS_FILES ts_subdir _sources)
    IF(XSLTPROC_EXECUTABLE AND LRELEASE_EXECUTABLE)
        FILE(GLOB ts_files ${ts_subdir}/avidemux_*.ts)

        FOREACH(ts_input ${ts_files})
            GET_FILENAME_COMPONENT(_in       ${ts_input} ABSOLUTE)
            GET_FILENAME_COMPONENT(_basename ${ts_input} NAME_WE)

            FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
            GET_FILENAME_COMPONENT(_outXml ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.xml ABSOLUTE)
            GET_FILENAME_COMPONENT(_out ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.qm ABSOLUTE)
            
            ADD_CUSTOM_COMMAND(
                OUTPUT ${_outXml}
                COMMAND ${CMAKE_COMMAND}
                    -E echo
                    "Generating" ${_outXml} "from" ${_in}
                COMMAND ${XSLTPROC_EXECUTABLE}
                    ${ts_subdir}/qt_filter_context.xslt ${_in}
                    > ${_outXml}
                DEPENDS ${_in}
            )
            
            ADD_CUSTOM_COMMAND(
                OUTPUT ${_out}
                COMMAND ${CMAKE_COMMAND}
                    -E echo
                    "Generating" ${_out} "from" ${_outXml}
                COMMAND ${LRELEASE_EXECUTABLE}
                    ${_outXml}
                    -qm ${_out}
                DEPENDS ${_in}
            )
                
            SET(qm_files ${qm_files} ${_outXml} ${_out})

			INSTALL(FILES ${_out} DESTINATION "${CMAKE_INSTALL_PREFIX}/${BIN_DIR}/i18n")
        ENDFOREACH(ts_input ${ts_files})

        SET(${_sources} ${${_sources}} ${qm_files})
    ENDIF(XSLTPROC_EXECUTABLE AND LRELEASE_EXECUTABLE)
ENDMACRO(COMPILE_AVIDEMUX_TS_FILES)

MACRO(COMPILE_QT_TS_FILES ts_subdir _sources)
    IF(XSLTPROC_EXECUTABLE AND LRELEASE_EXECUTABLE)
        FILE(GLOB ts_files ${ts_subdir}/qt_*.ts)

        FOREACH(ts_input ${ts_files})
            GET_FILENAME_COMPONENT(_in       ${ts_input} ABSOLUTE)
            GET_FILENAME_COMPONENT(_basename ${ts_input} NAME_WE)

            FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
            GET_FILENAME_COMPONENT(_out ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.qm ABSOLUTE)
            
            ADD_CUSTOM_COMMAND(
                OUTPUT ${_out}
                COMMAND ${CMAKE_COMMAND}
                    -E echo
                    "Generating" ${_out} "from" ${_in}
                COMMAND ${LRELEASE_EXECUTABLE}
                    ${_in}
                    -qm ${_out}
                DEPENDS ${_in}
            )
                
            SET(qm_files ${qm_files} ${_out})

			INSTALL(FILES ${_out} DESTINATION "${CMAKE_INSTALL_PREFIX}/${BIN_DIR}/i18n")
        ENDFOREACH(ts_input ${ts_files})

        SET(${_sources} ${${_sources}} ${qm_files})
    ENDIF(XSLTPROC_EXECUTABLE AND LRELEASE_EXECUTABLE)
ENDMACRO(COMPILE_QT_TS_FILES)

MACRO(COMPILE_TS_FILES ts_subdir _sources)
	FIND_XSLTPROC()
	FIND_LRELEASE()
	
	#COMPILE_AVIDEMUX_TS_FILES(${ts_subdir} ${_sources})
	#COMPILE_QT_TS_FILES(${ts_subdir} ${_sources})
ENDMACRO(COMPILE_TS_FILES)
