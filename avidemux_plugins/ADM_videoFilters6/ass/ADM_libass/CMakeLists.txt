INCLUDE(admCheckFontConfig)
checkFontConfig()

#CONFIG_FONTCONFIG
#CONFIG_ENCA
#CONFIG_ICONV
#
#
#

SET(ADM_LIB ADM_libass)

SET(${ADM_LIB}_SRCS 
ass_bitmap.c  ass.c  ass_cache.c  ass_fontconfig.c  ass_library.c   ass_render.c  ass_utils.c
ass_font.c ass_parse.c ass_drawing.c ass_render_api.c ass_strtod.c)
#  mputils.c

ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
ADD_DEFINITIONS(${FREETYPE2_CFLAGS} "-I${LIBICONV_INCLUDE_DIR}")

IF (FONTCONFIG_FOUND)
	ADD_DEFINITIONS(${FONTCONFIG_CFLAGS} "-DCONFIG_FONTCONFIG=1")
ENDIF (FONTCONFIG_FOUND)

# ?
ADD_DEFINITIONS(${FONTCONFIG_CFLAGS} "-DCONFIG_ICONV=1")
TARGET_LINK_LIBRARIES(${ADM_LIB} ${LIBICONV_LIBRARY_DIR})

IF (UNIX)
	ADD_TARGET_CFLAGS(${ADM_LIB} -fPIC)
ENDIF (UNIX)
