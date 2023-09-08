MACRO(checkAomDec)

MESSAGE(STATUS "Checking for libaom (decoder)")
MESSAGE(STATUS "*****************************")

IF (NOT AOM_CHECKED)
    OPTION(AOM "" ON)

    IF (AOM)
        PKG_CHECK_MODULES(AOM aom)

        IF (${AOM_FOUND})
            PRINT_LIBRARY_INFO("AOM" AOM_FOUND "${AOM_INCLUDEDIR}" "${AOM_LDFLAGS}")
        ENDIF (${AOM_FOUND})
    ENDIF (AOM)
    SET(AOM_CHECKED 1)
ENDIF (NOT AOM_CHECKED)

IF (${AOM_FOUND})
    ADM_CHECK_FUNCTION_EXISTS(aom_codec_dec_init_ver "${AOM_LDFLAGS}" AOM_DEC_INIT_FOUND "" "-I${AOM_INCLUDEDIR}")
    IF (${AOM_DEC_INIT_FOUND})
        SET(USE_AOM_DEC 1)
        MESSAGE(STATUS "Linking to AOM AV1 decoder library")
    ELSE (${AOM_DEC_INIT_FOUND})
        MESSAGE("${MSG_DISABLE_OPTION}")
    ENDIF (${AOM_DEC_INIT_FOUND})
ENDIF (${AOM_FOUND})
MESSAGE("")
APPEND_SUMMARY_LIST("Video Decoder" "libaom" "${USE_AOM_DEC}")

ENDMACRO(checkAomDec)

###############################################

MACRO(checkAomEnc)

MESSAGE(STATUS "Checking for libaom (encoder)")
MESSAGE(STATUS "*****************************")

IF (NOT AOM_VERSION_CHECKED)
    OPTION(AOM "" ON)

    IF (AOM)
        PKG_CHECK_MODULES(AOM aom>=3.2.0)

        IF (${AOM_FOUND})
            PRINT_LIBRARY_INFO("AOM" AOM_FOUND "${AOM_INCLUDEDIR}" "${AOM_LDFLAGS}")
        ENDIF (${AOM_FOUND})
    ENDIF (AOM)
    SET(AOM_VERSION_CHECKED 1)
ENDIF (NOT AOM_VERSION_CHECKED)

IF (${AOM_FOUND})
    ADM_CHECK_FUNCTION_EXISTS(aom_codec_enc_init_ver "${AOM_LDFLAGS}" AOM_ENC_INIT_FOUND "" "-I${AOM_INCLUDEDIR}")
    IF (${AOM_ENC_INIT_FOUND})
        SET(USE_AOM_ENC 1)
        MESSAGE(STATUS "Linking to AOM AV1 encoder library")
    ELSE (${AOM_ENC_INIT_FOUND})
        MESSAGE("${MSG_DISABLE_OPTION}")
    ENDIF (${AOM_ENC_INIT_FOUND})
ENDIF (${AOM_FOUND})
MESSAGE("")
APPEND_SUMMARY_LIST("Video Encoder" "libaom" "${USE_AOM_ENC}")
ENDMACRO(checkAomEnc)
