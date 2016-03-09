#
# Small macro to update the dependency list of .deb 
#
MACRO(SETDEBIANDEPS USE lib lst)
        MESSAGE(STATUS "Checking if we have ${USE} set")
        IF((${USE}))
                MESSAGE(STATUS "      yes")
                SET(${lst} "${${lst}}, ${lib}")
                #MESSAGE(STATUS "DEPS = ${${lst}}")
        ELSE((${USE}))
                MESSAGE(STATUS "      no")
        ENDIF((${USE}))
ENDMACRO(SETDEBIANDEPS USE lib lst)

