
  MACRO (QT4_EXTRACT_OPTIONS _qt4_files _qt4_options)
    SET(${_qt4_files})
    SET(${_qt4_options})
    SET(_QT4_DOING_OPTIONS FALSE)
    FOREACH(_currentArg ${ARGN})
      IF ("${_currentArg}" STREQUAL "OPTIONS")
        SET(_QT4_DOING_OPTIONS TRUE)
      ELSE ("${_currentArg}" STREQUAL "OPTIONS")
        IF(_QT4_DOING_OPTIONS) 
          LIST(APPEND ${_qt4_options} "${_currentArg}")
        ELSE(_QT4_DOING_OPTIONS)
          LIST(APPEND ${_qt4_files} "${_currentArg}")
        ENDIF(_QT4_DOING_OPTIONS)
      ENDIF ("${_currentArg}" STREQUAL "OPTIONS")
    ENDFOREACH(_currentArg) 
  ENDMACRO (QT4_EXTRACT_OPTIONS)
  
  # macro used to create the names of output files preserving relative dirs
  MACRO (QT4_MAKE_OUTPUT_FILE infile prefix ext outfile )
    STRING(LENGTH ${CMAKE_CURRENT_BINARY_DIR} _binlength)
    STRING(LENGTH ${infile} _infileLength)
    SET(_checkinfile ${CMAKE_CURRENT_SOURCE_DIR})
    IF(_infileLength GREATER _binlength)
      STRING(SUBSTRING "${infile}" 0 ${_binlength} _checkinfile)
      IF(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
        FILE(RELATIVE_PATH rel ${CMAKE_CURRENT_BINARY_DIR} ${infile})
      ELSE(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
        FILE(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})
      ENDIF(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    ELSE(_infileLength GREATER _binlength)
      FILE(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})
    ENDIF(_infileLength GREATER _binlength)
    SET(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${rel}")
    STRING(REPLACE ".." "__" _outfile ${_outfile})
    GET_FILENAME_COMPONENT(outpath ${_outfile} PATH)
    GET_FILENAME_COMPONENT(_outfile ${_outfile} NAME_WE)
    FILE(MAKE_DIRECTORY ${outpath})
    SET(${outfile} ${outpath}/${prefix}${_outfile}.${ext})
  ENDMACRO (QT4_MAKE_OUTPUT_FILE )

  MACRO (QT4_GET_MOC_FLAGS _moc_flags)
     SET(${_moc_flags})
     GET_DIRECTORY_PROPERTY(_inc_DIRS INCLUDE_DIRECTORIES)

     FOREACH(_current ${_inc_DIRS})
        SET(${_moc_flags} ${${_moc_flags}} "-I${_current}")
     ENDFOREACH(_current ${_inc_DIRS})
     
     GET_DIRECTORY_PROPERTY(_defines COMPILE_DEFINITIONS)
     FOREACH(_current ${_defines})
        SET(${_moc_flags} ${${_moc_flags}} "-D${_current}")
     ENDFOREACH(_current ${_defines})

     IF(Q_WS_WIN)
       SET(${_moc_flags} ${${_moc_flags}} -DWIN32)
     ENDIF(Q_WS_WIN)

  ENDMACRO(QT4_GET_MOC_FLAGS)

  # helper macro to set up a moc rule
  MACRO (QT4_CREATE_MOC_COMMAND infile outfile moc_flags moc_options)
    # For Windows, create a parameters file to work around command line length limit
    IF (WIN32)
      # Pass the parameters in a file.  Set the working directory to
      # be that containing the parameters file and reference it by
      # just the file name.  This is necessary because the moc tool on
      # MinGW builds does not seem to handle spaces in the path to the
      # file given with the @ syntax.
      GET_FILENAME_COMPONENT(_moc_outfile_name "${outfile}" NAME)
      GET_FILENAME_COMPONENT(_moc_outfile_dir "${outfile}" PATH)
      IF(_moc_outfile_dir)
        SET(_moc_working_dir WORKING_DIRECTORY ${_moc_outfile_dir})
      ENDIF(_moc_outfile_dir)
      SET (_moc_parameters_file ${outfile}_parameters)
      SET (_moc_parameters ${moc_flags} ${moc_options} -o "${outfile}" "${infile}")
      FILE (REMOVE ${_moc_parameters_file})
      FOREACH(arg ${_moc_parameters})
        FILE (APPEND ${_moc_parameters_file} "${arg}\n")
      ENDFOREACH(arg)
      ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
                         COMMAND ${QT_MOC_EXECUTABLE} @${_moc_outfile_name}_parameters
                         DEPENDS ${infile}
                         ${_moc_working_dir}
                         VERBATIM)
    ELSE (WIN32)     
      ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
                         COMMAND ${QT_MOC_EXECUTABLE}
                         ARGS ${moc_flags} ${moc_options} -o ${outfile} ${infile}
                         DEPENDS ${infile})     
    ENDIF (WIN32)
  ENDMACRO (QT4_CREATE_MOC_COMMAND)

  
  MACRO (QT4_GENERATE_MOC infile outfile )
     QT4_GET_MOC_FLAGS(moc_flags)
     GET_FILENAME_COMPONENT(abs_infile ${infile} ABSOLUTE)
     QT4_CREATE_MOC_COMMAND(${abs_infile} ${outfile} "${moc_flags}" "")
     SET_SOURCE_FILES_PROPERTIES(${outfile} PROPERTIES SKIP_AUTOMOC TRUE)  # dont run automoc on this file
  ENDMACRO (QT4_GENERATE_MOC)


  # QT4_WRAP_CPP(outfiles inputfile ... )

  MACRO (QT4_WRAP_CPP outfiles )
    # get include dirs
    QT4_GET_MOC_FLAGS(moc_flags)
    QT4_EXTRACT_OPTIONS(moc_files moc_options ${ARGN})

    FOREACH (it ${moc_files})
      GET_FILENAME_COMPONENT(it ${it} ABSOLUTE)
      QT4_MAKE_OUTPUT_FILE(${it} moc_ cxx outfile)
      QT4_CREATE_MOC_COMMAND(${it} ${outfile} "${moc_flags}" "${moc_options}")
      SET(${outfiles} ${${outfiles}} ${outfile})
    ENDFOREACH(it)

  ENDMACRO (QT4_WRAP_CPP)


  # QT4_WRAP_UI(outfiles inputfile ... )

  MACRO (QT4_WRAP_UI outfiles )
    QT4_EXTRACT_OPTIONS(ui_files ui_options ${ARGN})

    FOREACH (it ${ui_files})
      GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
      GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
      SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/ui_${outfile}.h)
      ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
        COMMAND ${QT_UIC_EXECUTABLE}
        ARGS ${ui_options} -o ${outfile} ${infile}
        MAIN_DEPENDENCY ${infile})
      SET(${outfiles} ${${outfiles}} ${outfile})
    ENDFOREACH (it)

  ENDMACRO (QT4_WRAP_UI)


  # QT4_ADD_RESOURCES(outfiles inputfile ... )

  MACRO (QT4_ADD_RESOURCES outfiles )
    QT4_EXTRACT_OPTIONS(rcc_files rcc_options ${ARGN})

    FOREACH (it ${rcc_files})
      GET_FILENAME_COMPONENT(outfilename ${it} NAME_WE)
      GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
      GET_FILENAME_COMPONENT(rc_path ${infile} PATH)
      SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/qrc_${outfilename}.cxx)
      #  parse file for dependencies 
      #  all files are absolute paths or relative to the location of the qrc file
      FILE(READ "${infile}" _RC_FILE_CONTENTS)
      STRING(REGEX MATCHALL "<file[^<]+" _RC_FILES "${_RC_FILE_CONTENTS}")
      SET(_RC_DEPENDS)
      FOREACH(_RC_FILE ${_RC_FILES})
        STRING(REGEX REPLACE "^<file[^>]*>" "" _RC_FILE "${_RC_FILE}")
        STRING(REGEX MATCH "^/|([A-Za-z]:/)" _ABS_PATH_INDICATOR "${_RC_FILE}")
        IF(NOT _ABS_PATH_INDICATOR)
          SET(_RC_FILE "${rc_path}/${_RC_FILE}")
        ENDIF(NOT _ABS_PATH_INDICATOR)
        SET(_RC_DEPENDS ${_RC_DEPENDS} "${_RC_FILE}")
      ENDFOREACH(_RC_FILE)
      ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
        COMMAND ${QT_RCC_EXECUTABLE}
        ARGS ${rcc_options} -name ${outfilename} -o ${outfile} ${infile}
        MAIN_DEPENDENCY ${infile}
        DEPENDS ${_RC_DEPENDS})
      SET(${outfiles} ${${outfiles}} ${outfile})
    ENDFOREACH (it)

  ENDMACRO (QT4_ADD_RESOURCES)

