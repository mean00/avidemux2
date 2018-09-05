MACRO(checkNvEnc)
	IF (NOT NVENC_CHECKED)
		OPTION(NVENC "" ON)

		MESSAGE(STATUS "Checking for NVENC")
		MESSAGE(STATUS "*****************")

		IF (NVENC)
            PKG_CHECK_MODULES(FFNVENC ffnvcodec)
		    IF (FFNVENC_FOUND)
                        FIND_PATH(NVENC_INCLUDE_DIR nvEncodeAPI.h
                        PATHS /usr/include /usr/include/nvenc /usr/include/x86_64-linux-gnu ${FFNVENC_CFLAGS}) # Needed for 64 bits linux
                        IF(NVENC_INCLUDE_DIR)
				        MESSAGE(STATUS " nvenc header Found in ${NVENC_INCLUDE_DIR}")
                                SET(USE_NVENC True)
                                SET(NVENC_FOUND 1)
                        ELSE(NVENC_INCLUDE_DIR)
				                MESSAGE(STATUS " nvenc header not Found ")
                                SET(NVENC_FOUND 0)
                        ENDIF(NVENC_INCLUDE_DIR)
		        SET(NVENC_CHECKED 1)
		    ELSE (FFNVENC_FOUND)
                MESSAGE(STATUS "FFNVENC not found, you can get it from here https://github.com/FFmpeg/nv-codec-headers")
		    ENDIF (FFNVENC_FOUND)
		ENDIF (NVENC)

		MESSAGE("")
	APPEND_SUMMARY_LIST("Video Encoder" "NVENC" "${NVENC_FOUND}")
	ENDIF (NOT NVENC_CHECKED)

ENDMACRO(checkNvEnc)
