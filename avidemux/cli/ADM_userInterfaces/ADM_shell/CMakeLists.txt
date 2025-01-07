SET(ADM_LIB ADM_shellCli)

SET(${ADM_LIB}_SRCS
        cliShell.cpp
)

ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
TARGET_LINK_LIBRARIES(${ADM_LIB} PRIVATE ADM_editor6_interface adm_gettext)
