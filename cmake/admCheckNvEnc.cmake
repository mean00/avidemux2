MACRO(checkNvEnc)
	IF (NOT NVENC_CHECKED)
		OPTION(NVENC "" ON)

		MESSAGE(STATUS "Checking for NVENC")
		MESSAGE(STATUS "*****************")

		IF (NVENC)
                        FIND_PATH(NVENC_INCLUDE_DIR nvEncodeAPI.h 
		      	PATHS /usr/include/x86_64-linux-gnu) # Needed for 64 bits linux
                        IF(NVENC_INCLUDE_DIR)
				MESSAGE(STATUS " nvenc header Found ")
                                SET(USE_NVENC True)
                                # DISABLED SET(NVENC_FOUND 1)
                                SET(NVENC_FOUND 0)
                        ELSE(NVENC_INCLUDE_DIR)
				MESSAGE(STATUS " nvenc header not Found ")
                                SET(NVENC_FOUND 0)
                        ENDIF(NVENC_INCLUDE_DIR)
		        SET(NVENC_CHECKED 1)
		ENDIF (NVENC)

		MESSAGE("")
	APPEND_SUMMARY_LIST("Video Encoder" "NVENC" "${NVENC_FOUND}")
	ENDIF (NOT NVENC_CHECKED)

ENDMACRO(checkNvEnc)
