#
   MESSAGE(STATUS "Checking for YASM")
   MESSAGE(STATUS "*****************")
   FIND_PROGRAM(YASM_YASM yasm)
   IF(YASM_YASM STREQUAL "<YASM_YASM>-NOTFOUND")
       MESSAGE(FATAL_ERROR "Yasm Not found. Stopping here.")
   ENDIF(YASM_YASM STREQUAL "<YASM_YASM>-NOTFOUND")
   MESSAGE(STATUS "Found as ${YASM_YASM}")

   SET(ASM_DIALECT "_NASM")

   SET(CMAKE_ASM_SOURCE_FILE_EXTENSIONS nasm;nas;asm)
   ENABLE_LANGUAGE(ASM_NASM)
   SET(CMAKE_ASM_NASM_COMPILER ${YASM_YASM})
   SET(ASM_ARGS "")
   SET(ASM_ARGS_FLAGS  -DHAVE_ALIGNED_STACK=1  -DHAVE_CPUNOP=0)
   SET(ASM_ARGS_FORMAT -f win64 -m amd64 -DARCH_X86_64=1 -DARCH_X86_32=0)

   IF(AVIDEMUX_THIS_IS_CORE)
        SET(NASM_MACRO_FOLDER  ${AVIDEMUX_TOP_SOURCE_DIR}/avidemux_core/ADM_core/include)
   ELSE(AVIDEMUX_THIS_IS_CORE)
        SET(NASM_MACRO_FOLDER  ${ADM_HEADER_DIR}/ADM_core/)
   ENDIF(AVIDEMUX_THIS_IS_CORE)


    
    SET(CMAKE_ASM_NASM_FLAGS ${ASM_ARGS_FORMAT} ${ASM_ARGS_FLAGS} -I${NASM_MACRO_FOLDER}  )
    INCLUDE(CMakeASMInformation)
    SET( CMAKE_ASM_NASM_COMPILE_OBJECT  "<CMAKE_ASM_NASM_COMPILER> ${CMAKE_ASM_NASM_FLAGS} -o <OBJECT> <SOURCE>" CACHE INTERNAL "")




MACRO(YASMIFY out filez)
           MESSAGE(STATUS "YASMIFY : ${filez}")
           foreach(fl ${filez})
             MESSAGE(STATUS "    ${CMAKE_CURRENT_SOURCE_DIR}/${fl}.asm ==> ${CMAKE_CURRENT_BINARY_DIR}/${fl}.obj")
             add_custom_command(
                 OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${fl}.obj
                 COMMAND ${CMAKE_COMMAND} -E echo   ${YASM_YASM} ARGS ${ASM_ARGS_FORMAT} ${ASM_ARGS_FLAGS} -I${NASM_MACRO_FOLDER}  -o ${CMAKE_CURRENT_BINARY_DIR}/${fl}.obj ${CMAKE_CURRENT_SOURCE_DIR}/${fl}.asm
                 COMMAND ${YASM_YASM} ARGS ${ASM_ARGS_FORMAT} ${ASM_ARGS_FLAGS} -I${NASM_MACRO_FOLDER}  -o ${CMAKE_CURRENT_BINARY_DIR}/${fl}.obj ${CMAKE_CURRENT_SOURCE_DIR}/${fl}.asm
                 DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${fl}.asm)
                 LIST( APPEND ${out} ${CMAKE_CURRENT_BINARY_DIR}/${fl}.obj)
            endforeach(fl ${filez})
        MESSAGE(STATUS "ASM files : ${${out}}")
ENDMACRO(YASMIFY out filez)


#
#
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
    #message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

