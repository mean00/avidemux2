# - Extract information from a subversion working copy
MACRO(admFindYasm )

FIND_PROGRAM(YASM_EXECUTABLE yasm
  DOC "yasm command line assembler")
MARK_AS_ADVANCED(YASM_EXECUTABLE)

IF(YASM_EXECUTABLE)
        
        SET(EXE  "cd ${_dir}&& ${GIT_EXECUTABLE} svn log | head -2  | grep '^r' | sed 's/ .*$//g'" )
        MESSAGE(STATUS "Found yasm")
        SET(USE_YASM 1)
        IF(WIN32)
                SET(YASM_FLAGS -f coff -DPREFIX)
        ELSE(WIN32)
                SET(YASM_FLAGS -f elf -DPIC -g dwarf2)
        ENDIF(WIN32)
        IF(ADM_CPU_X86_64)
                SET(YASM_FLAGS ${YASM_FLAGS} -DARCH_X86_64 -m amd64)
        ELSE(ADM_CPU_X86_64)
                SET(YASM_FLAGS ${YASM_FLAGS} -DARCH_X86_32)
        ENDIF(ADM_CPU_X86_64)
        MESSAGE(STATUS "Found yasm, flags ${YASM_FLAGS}")
MACRO(YASM_COMPILE _in _out _inc)
        ADD_CUSTOM_COMMAND(
                OUTPUT ${_out}
                COMMAND ${CMAKE_COMMAND} -E echo "Generating" ${_out} "from" ${_in}
                COMMAND ${YASM_EXECUTABLE} -i ${_inc}  ${YASM_FLAGS} ${_in} -o ${_out}
                DEPENDS ${_in}
                VERBATIM
                )
ENDMACRO(YASM_COMPILE)

ENDIF(YASM_EXECUTABLE)

ENDMACRO(admFindYasm )

# 
