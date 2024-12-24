include(GetPrerequisites)  


MACRO(checkVapourSynth)
    macro(GET_VAPOURSYNTH_PYTHON lib result)
        SET(${result} "NOTFOUND")
        GET_PREREQUISITES(${lib}  xoutput 0 0 "" "")
        FOREACH(dep ${xoutput})
            if(dep MATCHES ".*libpython3.*")
                # path looks like /usr/lib/x86_64-linux-gnu/libpython3.6m.so.1.0
                STRING(REGEX REPLACE "^.*libpython3" "libpython3" shortened "${dep}" )
                STRING(REGEX REPLACE "so.*$" "so" shortened "${shortened}" )
                # Now it looks like this libpython3.6m.so
                SET(${result} ${shortened})
                MESSAGE(STATUS "Got python lib :  ${${result}}")
            ENDIF()
        ENDFOREACH(dep ${xoutput})
    ENDMACRO()

	IF(NOT VAPOURSYNTH_CHECKED)
		OPTION(VAPOURSYNTH "" ON)

		MESSAGE(STATUS "Checking for VAPOURSYNTH")
		MESSAGE(STATUS "***********************")

		IF(VAPOURSYNTH)
                        PKG_CHECK_MODULES(VAPOURSYNTHSCRIPT vapoursynth-script)
                        IF(VAPOURSYNTHSCRIPT_FOUND)
                                MESSAGE(STATUS "VSSCRIPT<${VAPOURSYNTHSCRIPT_CFLAGS}>  < ${VAPOURSYNTHSCRIPT_LDFLAGS}> <${VAPOURSYNTHSCRIPT_LIBRARIES}>")
                                IF(NOT APPLE AND NOT WIN32)
                                    LIST(GET VAPOURSYNTHSCRIPT_LIBRARIES 0  xfirst)
                                    GET_VAPOURSYNTH_PYTHON( "${VAPOURSYNTHSCRIPT_LIBDIR}/lib${xfirst}.so" VAPOURSYNTH_PYTHONLIB)
                                    SET(VAPOURSYNTHSCRIPT_CFLAGS "${VAPOURSYNTHSCRIPT_CFLAGS} -DVAPOURSYNTH_PYTHONLIB='\"${VAPOURSYNTH_PYTHONLIB}\"'")
                                ENDIF()
                                SET(USE_VAPOURSYNTH 1)
                        ENDIF()
		ELSE()
			MESSAGE("${MSG_DISABLE_OPTION}")
		ENDIF()

		SET(VAPOURSYNTH_CHECKED 1)

		MESSAGE("")
	ENDIF()

	APPEND_SUMMARY_LIST("Miscellaneous" "VapourSynth" "${VAPOURSYNTHSCRIPT_FOUND}")
ENDMACRO()
