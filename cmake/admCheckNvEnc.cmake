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
                        ELSE(NVENC_INCLUDE_DIR)
				MESSAGE(STATUS " nvenc header not Found ")
                        ENDIF(NVENC_INCLUDE_DIR)
		        SET(NVENC_CHECKED 1)
		ENDIF (NVENC)

		MESSAGE("")
	ENDIF (NOT NVENC_CHECKED)

	APPEND_SUMMARY_LIST("Video Encoder" "NVENC" "${NVENC_FOUND}")
ENDMACRO(checkNvEnc)
