# Outputs:
#   FDK_AAC_INCLUDEDIR
#   FDK_AAC_LINK_LIBRARIES
#   ENV{ADM_HAVE_FDK_AAC}

MACRO(checkFdkAAC)
    IF (NOT FDK_AAC_CHECKED)
        OPTION (FDK_AAC "" ON)

        MESSAGE(STATUS "Checking for FDK_AAC")
        MESSAGE(STATUS "*****************")

        IF (FDK_AAC)
            #FIND_HEADER_AND_LIB(FDK_AAC fdk-aac/aacenc_lib.h fdk-aac) # Use pkg config ?
            PKG_CHECK_MODULES(FDK_AAC fdk-aac)
#            DUMP_ALL_VARS()

            IF (FDK_AAC_FOUND)
                PRINT_LIBRARY_INFO("FDK_AAC" FDK_AAC_FOUND "${FDK_AAC_INCLUDEDIR}" "${FDK_AAC_LDFLAGS}")
                ADM_CHECK_FUNCTION_EXISTS(aacEncOpen "${FDK_AAC_LDFLAGS}" FDK_OPEN_FUNCTION_FOUND "" -I"${FDK_AAC_INCLUDEDIR}")
                IF (FDK_OPEN_FUNCTION_FOUND)
                        SET(FDK_AAC_FOUND 1)
                        SET(USE_FDK_AAC  1)
                ENDIF (FDK_OPEN_FUNCTION_FOUND)
            ENDIF (FDK_AAC_FOUND)
        ELSE (FDK_AAC)
            MESSAGE("${MSG_DISABLE_OPTION}")
        ENDIF (FDK_AAC)

        SET(FDK_AAC_CHECKED 1)

        MESSAGE("")
    ENDIF (NOT FDK_AAC_CHECKED)

    APPEND_SUMMARY_LIST("Audio Encoder" "FDK_AAC" "${FDK_AAC_FOUND}")
ENDMACRO(checkFdkAAC)
