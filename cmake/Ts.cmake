# a small macro to create qm files out of ts's

MACRO(FIND_LRELEASE)
    IF(NOT LRELEASE_EXECUTABLE AND NOT LRELEASE_NOT_FOUND)
        FIND_PROGRAM(LRELEASE_EXECUTABLE lrelease PATHS
            "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
            "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
            $ENV{QTDIR}/bin)

        IF (NOT LRELEASE_EXECUTABLE) # search again under the name lrelease-qt4
                FIND_PROGRAM(LRELEASE_EXECUTABLE lrelease-${QT_EXTENSION} PATHS
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
MACRO(INSTALL_I18N )
        IF(WIN32)
                SET(i18dir "${CMAKE_INSTALL_PREFIX}/${BIN_DIR}/${QT_EXTENSION}/i18n")
        ELSE(WIN32)
                SET(i18dir "${CMAKE_INSTALL_PREFIX}/share/avidemux6/${QT_EXTENSION}/i18n")
        ENDIF(WIN32)
        INSTALL(FILES ${ARGN} DESTINATION "${i18dir}" COMPONENT runtime)
ENDMACRO(INSTALL_I18N _files)
#
#
FUNCTION(COMPILE_MASK_TS_FILES ts_subdir prefix mask_sources)
    IF(LRELEASE_EXECUTABLE)
        FILE(GLOB ts_files ${ts_subdir}/${prefix}_*.ts)
        #MESSAGE(STATUS "While searching ${prefix} found ${ts_files}")
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
        ENDFOREACH(ts_input ${ts_files})
        INSTALL_I18N( ${qm_files})
        SET(${mask_sources}  ${qm_files} PARENT_SCOPE)
    ENDIF(LRELEASE_EXECUTABLE)
ENDFUNCTION(COMPILE_MASK_TS_FILES)
#
#
#
FUNCTION(COMPILE_TS_FILES ts_subdir sources)
  FIND_LRELEASE()
  IF(QT5_FOUND)
      SET(pfix "qtbase")
  ELSE(QT5_FOUND)
      SET(pfix "qt")
  ENDIF(QT5_FOUND)
  SET(adm_sources "")
  COMPILE_MASK_TS_FILES( ${ts_subdir} "avidemux" adm_sources)
  #MESSAGE(STATUS "Result 1= ${adm_sources}")
  SET(qt_sources "")
  COMPILE_MASK_TS_FILES( ${ts_subdir} "${pfix}"    qt_sources)
  #MESSAGE(STATUS "Result 2= ${qt_sources}")
  SET( ${sources} ${adm_sources} ${qt_sources} PARENT_SCOPE)
ENDFUNCTION(COMPILE_TS_FILES)
