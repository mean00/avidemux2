# a small macro to create mo files out of po's
# Taken from sim-im.org

MACRO(FIND_MSGFMT)
    IF(NOT MSGFMT_EXECUTABLE)
        IF(NOT MSGFMT_NOT_FOUND)
            SET(MSGFMT_NAME "msgfmt")
            FIND_PROGRAM(MSGFMT_EXECUTABLE ${MSGFMT_NAME})

            IF(NOT MSGFMT_EXECUTABLE)
              MESSAGE(STATUS "WARNING: ${MSGFMT_NAME} not found - po files can't be processed")
              SET(MSGFMT_NOT_FOUND "1")     # to avoid double checking in one cmake run
            ENDIF()

            MARK_AS_ADVANCED(MSGFMT_EXECUTABLE)
        ENDIF()
    ENDIF()
ENDMACRO()

MACRO(COMPILE_PO_FILES po_subdir _sources)
    FIND_MSGFMT()

    IF(MSGFMT_EXECUTABLE)
        FILE(GLOB po_files ${po_subdir}/*.po)

        FOREACH(po_input ${po_files})
            GET_FILENAME_COMPONENT(_in ${po_input} ABSOLUTE)
            GET_FILENAME_COMPONENT(_basename ${po_input} NAME_WE)

            FILE(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_basename}")
            GET_FILENAME_COMPONENT(_out "${CMAKE_CURRENT_BINARY_DIR}/${_basename}/avidemux.mo" ABSOLUTE)

            ADD_CUSTOM_COMMAND(
                OUTPUT ${_out}
                COMMAND ${CMAKE_COMMAND} -E echo "Generating" ${_out} "from" ${_in}
                COMMAND ${MSGFMT_EXECUTABLE} ${_in} -o ${_out}
                DEPENDS ${_in})

            SET(mo_files ${mo_files} ${_out})

            INSTALL(FILES ${_out} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/locale/${_basename}/LC_MESSAGES")
        ENDFOREACH(po_input ${po_files})

        SET(${_sources} ${${_sources}} ${mo_files})
    ENDIF()
ENDMACRO()
