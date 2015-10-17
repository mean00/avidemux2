# a small macro to create qm files out of ts's

MACRO(FIND_LRELEASE)
    IF(NOT LRELEASE_EXECUTABLE AND NOT LRELEASE_NOT_FOUND)
		FIND_PROGRAM(LRELEASE_EXECUTABLE lrelease PATHS
			"[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
			"[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
			$ENV{QTDIR}/bin)

		IF (NOT LRELEASE_EXECUTABLE) # search again under the name lrelease-qt4
        		FIND_PROGRAM(LRELEASE_EXECUTABLE lrelease-qt4 PATHS
			$ENV{QTDIR}/bin)
		        IF (NOT LRELEASE_EXECUTABLE) # search again under the name lrelease-qt4
			        MESSAGE(FATAL_ERROR "${LRELEASE_EXECUTABLE} not found - ts files can't be processed")
			        SET(LRELEASE_NOT_FOUND "1")     # to avoid double checking in one cmake run
		        ENDIF (NOT LRELEASE_EXECUTABLE) # search again under the name lrelease-qt4
		ENDIF (NOT LRELEASE_EXECUTABLE)
		IF (LRELEASE_EXECUTABLE) 
                        MESSAGE(STATUS "lrelease found as ${LRELEASE_EXECUTABLE}")
		ENDIF (LRELEASE_EXECUTABLE) 

	ENDIF(NOT LRELEASE_EXECUTABLE AND NOT LRELEASE_NOT_FOUND)
ENDMACRO(FIND_LRELEASE)
#
#
#
MACRO(INSTALL_I18N _files)
        IF(WIN32)
                SET(i18dir "${CMAKE_INSTALL_PREFIX}/${BIN_DIR}/${QT_EXTENSION}/i18n")
        ELSE(WIN32)
                SET(i18dir "${CMAKE_INSTALL_PREFIX}/share/avidemux6/${QT_EXTENSION}/i18n")
        ENDIF(WIN32)
        INSTALL(FILES ${_files} DESTINATION "${i18dir}" COMPONENT runtime)
ENDMACRO(INSTALL_I18N _files)
#
#
#
MACRO(COMPILE_AVIDEMUX_TS_FILES ts_subdir _sources)
    IF(LRELEASE_EXECUTABLE)
        FILE(GLOB ts_files ${ts_subdir}/avidemux_*.ts)

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
            INSTALL_I18N( ${_out})
        ENDFOREACH(ts_input ${ts_files})

        SET(${_sources} ${${_sources}} ${qm_files})
    ENDIF(LRELEASE_EXECUTABLE)
ENDMACRO(COMPILE_AVIDEMUX_TS_FILES)

MACRO(COMPILE_QT_TS_FILES ts_subdir _sources)
    IF(LRELEASE_EXECUTABLE)
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

            INSTALL_I18N( ${_out})
        ENDFOREACH(ts_input ${ts_files})

        SET(${_sources} ${${_sources}} ${qm_files})
    ENDIF(LRELEASE_EXECUTABLE)
ENDMACRO(COMPILE_QT_TS_FILES)

MACRO(COMPILE_TS_FILES ts_subdir _sources)
	FIND_LRELEASE()
	
	COMPILE_AVIDEMUX_TS_FILES(${ts_subdir} ${_sources})
	COMPILE_QT_TS_FILES(${ts_subdir} ${_sources})
ENDMACRO(COMPILE_TS_FILES)
