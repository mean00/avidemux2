INCLUDE(admCheckFreeType)
INCLUDE(admCheckFridibi)
checkFreeType()
checkFridibi("0.19")

IF (USE_FREETYPE AND USE_FRIDIBI)
	ADD_SUBDIRECTORY(ADM_libass)

	INCLUDE(vf_plugin)
	SET(ADM_vf_ssa_SRCS ADM_vidASS.cpp)

	ADD_VIDEO_FILTER(ADM_vf_ssa ${ADM_vf_ssa_SRCS})

	IF(DO_COMMON)
		TARGET_LINK_LIBRARIES(ADM_vf_ssa ADM_libass ${FREETYPE2_LDFLAGS} ${FRIDIBI_LDFLAGS})
                # We force the use of fontconfig
                ADD_DEFINITIONS("-DUSE_FONTCONFIG")
                TARGET_LINK_LIBRARIES(ADM_vf_ssa ${FONTCONFIG_LDFLAGS})
	ENDIF(DO_COMMON)

	INIT_VIDEO_FILTER(ADM_vf_ssa)
	INSTALL_VIDEO_FILTER(ADM_vf_ssa)
ENDIF (USE_FREETYPE AND USE_FRIDIBI)
